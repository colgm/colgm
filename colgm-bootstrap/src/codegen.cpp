#include "codegen.h"

namespace colgm {

bool codegen::visit_identifier(identifier* node) {
    out << node->get_name();
    return true;
}

bool codegen::visit_type_def(type_def* node) {
    node->get_name()->accept(this);
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
    out << "%struct." << node->get_name() << " = type {";
    for(auto i : node->get_fields()) {
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
    out << " @" << node->get_name();
    node->get_params()->accept(this);
    out << " {\n";
    out << "  ret ";
    node->get_return_type()->accept(this);
    out << " 0\n";
    out << "}\n\n";
    return true;
}

}