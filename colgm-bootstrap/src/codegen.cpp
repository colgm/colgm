#include "codegen.h"

namespace colgm {

bool codegen::visit_root(root* node) {
    structs = {};
    constant_strings = {};
    for(auto i : node->get_decls()) {
        i->accept(this);
    }
    return true;
}

bool codegen::visit_identifier(identifier* node) {
    out << node->get_name();
    return true;
}

bool codegen::visit_string_literal(string_literal* node) {
    constant_strings.push_back(node->get_string());
    return true;
}

bool codegen::visit_type_def(type_def* node) {
    const auto& type_name = node->get_name()->get_name();
    if (ctx->get_structs().count(type_name)) {
        out << "%struct." << type_name;
    } else {
        out << (
            basic_type_convert_mapper.count(type_name)?
            basic_type_convert_mapper.at(type_name):type_name
        );
    }
    for(size_t i = 0; i<node->get_pointer_level(); ++i) {
        out << "*";
    }
    return true;
}

bool codegen::visit_struct_field(struct_field* node) {
    node->get_type()->accept(this);
    return true;
}

bool codegen::visit_struct_decl(struct_decl* node) {
    structs.insert({node->get_name(), {}});
    structs.at(node->get_name()).name = "%struct." + node->get_name();
    out << "%struct." << node->get_name() << " = type {";
    for(auto i : node->get_fields()) {
        structs.at(node->get_name()).field.push_back({
            i->get_name()->get_name(),
            i->get_type()->get_name()->get_name(),
            i->get_type()->get_pointer_level()
        });
        i->accept(this);
        if (i!=node->get_fields().back()) {
            out << ", ";
        }
    }
    out << "}\n\n";
    return true;
}

bool codegen::visit_param(param* node) {
    node->get_type()->accept(this);
    out << " %";
    node->get_name()->accept(this);
    return true;
}

bool codegen::visit_param_list(param_list* node) {
    out << "(";
    for(auto i : node->get_params()) {
        i->accept(this);
        if (i!=node->get_params().back()) {
            out << ", ";
        }
    }
    out << ")";
    return true;
}

bool codegen::visit_func_decl(func_decl* node) {
    out << "define ";
    node->get_return_type()->accept(this);
    out << " @";
    if (!impl_struct_name.empty()) {
        out << impl_struct_name << ".";
    }
    out << node->get_name();
    node->get_params()->accept(this);
    out << " {\n";
    out << "  ret ";
    node->get_return_type()->accept(this);
    if (node->get_return_type()->get_name()->get_name()!="void") {
        if (node->get_return_type()->get_pointer_level()) {
            out << " null\n";
        } else {
            out << " 0\n";
        }
    } else {
        out << "\n";
    }
    out << "}\n\n";
    return true;
}

bool codegen::vissit_impl_struct(impl_struct* node) {
    impl_struct_name = node->get_struct_name();
    for(auto i : node->get_methods()) {
        i->accept(this);
    }
    impl_struct_name = "";
    return true;
}

void codegen::generate(root* ast_root, semantic* sema) {
    ctx = sema;
    ast_root->accept(this);
}

}