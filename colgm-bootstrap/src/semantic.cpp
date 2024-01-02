#include "semantic.h"

namespace colgm {

std::ostream& operator<<(std::ostream& out, const symbol& s) {
    out << s.type;
    for(uint64_t i = 0; i < s.type_pointer_level; ++i) {
        out << "*";
    }
    return out;
}

void semantic::analyse_single_struct(struct_decl* node) {
    if (structs.count(node->get_name())) {
        err.err(
            "sema", node->get_location(),
            "struct \"" + node->get_name() + "\" already exists."
        );
        return;
    }
    structs.insert({node->get_name(), {}});
    auto& struct_self = structs.at(node->get_name());
    for(auto i : node->get_fields()) {
        if (!symbols.count(i->get_type()->get_name()->get_name())) {
            err.err(
                "sema", i->get_type()->get_location(),
                "undefined symbol."
            );
        }
        struct_self.field.push_back({
            i->get_name()->get_name(),
            i->get_type()->get_name()->get_name(),
            i->get_type()->get_pointer_level()
        });
    }
}

void semantic::analyse_structs(root* ast_root) {
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_struct_decl) {
            continue;
        }
        auto struct_decl_node = reinterpret_cast<struct_decl*>(i);
        symbols.insert({struct_decl_node->get_name(), symbol_kind::struct_kind});
        analyse_single_struct(struct_decl_node);
    }
}

void semantic::analyse_single_func(func_decl* node) {
    if (functions.count(node->get_name())) {
        err.err(
            "sema", node->get_location(),
            "function \"" + node->get_name() + "\" already exists."
        );
    }
    functions.insert({node->get_name(), {}});
    auto& func_self = functions.at(node->get_name());
    for(auto i : node->get_params()->get_params()) {
        if (!symbols.count(i->get_type()->get_name()->get_name())) {
            err.err(
                "sema", i->get_type()->get_location(),
                "undefined symbol."
            );
        }
        func_self.parameters.push_back({
            i->get_name()->get_name(),
            i->get_type()->get_name()->get_name(),
            i->get_type()->get_pointer_level()
        });
    }
    if (!symbols.count(node->get_return_type()->get_name()->get_name())) {
        err.err(
            "sema", node->get_return_type()->get_location(),
            "undefined symbol."
        );
    }
    func_self.return_type = {
        "",
        node->get_return_type()->get_name()->get_name(),
        node->get_return_type()->get_pointer_level()
    };
}

void semantic::analyse_functions(root* ast_root) {
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_func_decl) {
            continue;
        }
        auto func_decl_node = reinterpret_cast<func_decl*>(i);
        symbols.insert({func_decl_node->get_name(), symbol_kind::func_kind});
        analyse_single_func(func_decl_node);
    }
}

const error& semantic::analyse(root* ast_root) {
    symbols.clear();
    symbols.insert({"i64", symbol_kind::basic_kind});
    symbols.insert({"i32", symbol_kind::basic_kind});
    symbols.insert({"i16", symbol_kind::basic_kind});
    symbols.insert({"i8", symbol_kind::basic_kind});
    symbols.insert({"u64", symbol_kind::basic_kind});
    symbols.insert({"u32", symbol_kind::basic_kind});
    symbols.insert({"u16", symbol_kind::basic_kind});
    symbols.insert({"u8", symbol_kind::basic_kind});
    symbols.insert({"f32", symbol_kind::basic_kind});
    symbols.insert({"f64", symbol_kind::basic_kind});
    symbols.insert({"void", symbol_kind::basic_kind});

    structs.clear();
    analyse_structs(ast_root);

    functions.clear();
    analyse_functions(ast_root);
    return err;
}

void semantic::dump() {
    for(const auto& i : structs) {
        std::cout << "struct " << i.first << "{\n";
        for(const auto& field : i.second.field) {
            std::cout << "  " << field.name << ": " << field << ",\n";
        }
        std::cout << "}\n";
    }
    for(const auto& i : functions) {
        std::cout << "func " << i.first << "(";
        for(const auto& param : i.second.parameters) {
            std::cout << param.name << ": " << param << ", ";
        }
        std::cout << ") -> " << i.second.return_type << "\n";
    }
}

}