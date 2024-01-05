#include "semantic.h"

namespace colgm {

std::ostream& operator<<(std::ostream& out, const symbol& s) {
    out << s.type;
    for(uint64_t i = 0; i < s.type_pointer_level; ++i) {
        out << "*";
    }
    return out;
}

std::string semantic::generate_type_string(type_def* node) {
    auto sym = symbol({
        "",
        node->get_name()->get_name(),
        node->get_pointer_level()
    });
    if (structs.count(sym.type)) {
        sym.type = "%struct." + sym.type;
    } else if (basic_type_convert_mapper.count(sym.type)) {
        sym.type = basic_type_convert_mapper.at(sym.type);
    }
    return sym.type_to_string();
}

bool semantic::visit_struct_decl(struct_decl* node) {
    auto new_ir = new ir_struct(node->get_name());
    for(auto i : node->get_fields()) {
        new_ir->add_field_type(generate_type_string(i->get_type()));
    }
    emit(new_ir);
    return true;
}

bool semantic::visit_func_decl(func_decl* node) {
    auto name = node->get_name();
    if (impl_struct_name.length()) {
        name = impl_struct_name + "." + name;
    }
    auto new_ir = new ir_func_decl("@" + name);
    new_ir->set_return_type(generate_type_string(node->get_return_type()));
    for(auto i : node->get_params()->get_params()) {
        new_ir->add_param(
            i->get_name()->get_name(),
            generate_type_string(i->get_type())
        );
    }
    emit(new_ir);
    return true;
}

bool semantic::visit_impl_struct(impl_struct* node) {
    impl_struct_name = node->get_struct_name();
    for(auto i : node->get_methods()) {
        i->accept(this);
    }
    impl_struct_name = "";
    return true;
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

colgm_func semantic::analyse_single_func(func_decl* node) {
    auto func_self = colgm_func();
    for(auto i : node->get_params()->get_params()) {
        if (!symbols.count(i->get_type()->get_name()->get_name())) {
            err.err("sema", i->get_type()->get_location(),
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
        err.err("sema", node->get_return_type()->get_location(),
            "undefined symbol."
        );
    }
    func_self.return_type = {
        "",
        node->get_return_type()->get_name()->get_name(),
        node->get_return_type()->get_pointer_level()
    };
    return func_self;
}

void semantic::analyse_functions(root* ast_root) {
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_func_decl) {
            continue;
        }
        auto func_decl_node = reinterpret_cast<func_decl*>(i);
        symbols.insert({func_decl_node->get_name(), symbol_kind::func_kind});
        if (functions.count(func_decl_node->get_name())) {
            err.err("sema", func_decl_node->get_location(),
                "function \"" + func_decl_node->get_name() + "\" already exists."
            );
            continue;
        }
        functions.insert({
            func_decl_node->get_name(),
            analyse_single_func(func_decl_node)
        });
    }
}

void semantic::analyse_single_impl(impl_struct* node) {
    if (!structs.count(node->get_struct_name())) {
        err.err("sema", node->get_location(),
            "undefined struct \"" + node->get_struct_name() + "\"."
        );
        return;
    }
    auto& stct = structs.at(node->get_struct_name());
    for(auto i : node->get_methods()) {
        if (stct.method.count(i->get_name())) {
            err.err("sema", i->get_location(),
                "method \"" + i->get_name() + "\" already exists."
            );
            continue;
        }
        stct.method.insert({i->get_name(), analyse_single_func(i)});
    }
}

void semantic::analyse_impls(root* ast_root) {
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_impl) {
            continue;
        }
        auto impl_node = reinterpret_cast<impl_struct*>(i);
        analyse_single_impl(impl_node);
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

    analyse_impls(ast_root);
    ast_root->accept(this);
    return err;
}

void semantic::dump() {
    for(const auto& i : structs) {
        std::cout << "struct " << i.first << " {\n";
        for(const auto& field : i.second.field) {
            std::cout << "  " << field.name << ": " << field;
            if (field.name!=i.second.field.back().name) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "}\n";
        if (i.second.method.empty()) {
            continue;
        }
        std::cout << "impl " << i.first << " {\n";
        for(const auto& method : i.second.method) {
            std::cout << "  func " << method.first << "(";
            for(const auto& param : method.second.parameters) {
                std::cout << param.name << ": " << param;
                if (param.name!=method.second.parameters.back().name) {
                    std::cout << ", ";
                }
            }
            std::cout << ") -> " << method.second.return_type << "\n";
        }
        std::cout << "}\n";
    }
    for(const auto& i : functions) {
        std::cout << "func " << i.first << "(";
        for(const auto& param : i.second.parameters) {
            std::cout << param.name << ": " << param;
            if (param.name!=i.second.parameters.back().name) {
                std::cout << ", ";
            }
        }
        std::cout << ") -> " << i.second.return_type << "\n";
    }
}

void semantic::dump_code() {
    for(auto i : generated_codes) {
        i->dump(std::cout);
    }
}

}