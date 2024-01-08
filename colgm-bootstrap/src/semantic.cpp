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
    if (ctx.structs.count(node->get_name())) {
        err.err(
            "sema", node->get_location(),
            "struct \"" + node->get_name() + "\" already exists."
        );
        return;
    }
    ctx.structs.insert({node->get_name(), {}});
    auto& struct_self = ctx.structs.at(node->get_name());
    for(auto i : node->get_fields()) {
        if (!ctx.symbols.count(i->get_type()->get_name()->get_name())) {
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
        ctx.symbols.insert({struct_decl_node->get_name(), symbol_kind::struct_kind});
        analyse_single_struct(struct_decl_node);
    }
}

colgm_func semantic::analyse_single_func(func_decl* node) {
    auto func_self = colgm_func();
    for(auto i : node->get_params()->get_params()) {
        if (!ctx.symbols.count(i->get_type()->get_name()->get_name())) {
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
    if (!ctx.symbols.count(node->get_return_type()->get_name()->get_name())) {
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
        ctx.symbols.insert({func_decl_node->get_name(), symbol_kind::func_kind});
        if (ctx.functions.count(func_decl_node->get_name())) {
            err.err("sema", func_decl_node->get_location(),
                "function \"" + func_decl_node->get_name() + "\" already exists."
            );
            continue;
        }
        ctx.functions.insert({
            func_decl_node->get_name(),
            analyse_single_func(func_decl_node)
        });
    }
}

void semantic::analyse_single_impl(impl_struct* node) {
    if (!ctx.structs.count(node->get_struct_name())) {
        err.err("sema", node->get_location(),
            "undefined struct \"" + node->get_struct_name() + "\"."
        );
        return;
    }
    auto& stct = ctx.structs.at(node->get_struct_name());
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
    ctx.symbols.clear();
    ctx.symbols.insert({"i64", symbol_kind::basic_kind});
    ctx.symbols.insert({"i32", symbol_kind::basic_kind});
    ctx.symbols.insert({"i16", symbol_kind::basic_kind});
    ctx.symbols.insert({"i8", symbol_kind::basic_kind});
    ctx.symbols.insert({"u64", symbol_kind::basic_kind});
    ctx.symbols.insert({"u32", symbol_kind::basic_kind});
    ctx.symbols.insert({"u16", symbol_kind::basic_kind});
    ctx.symbols.insert({"u8", symbol_kind::basic_kind});
    ctx.symbols.insert({"f32", symbol_kind::basic_kind});
    ctx.symbols.insert({"f64", symbol_kind::basic_kind});
    ctx.symbols.insert({"void", symbol_kind::basic_kind});
    ctx.symbols.insert({"bool", symbol_kind::basic_kind});

    ctx.structs.clear();
    analyse_structs(ast_root);

    ctx.functions.clear();
    analyse_functions(ast_root);

    analyse_impls(ast_root);
    return err;
}

void semantic::dump() {
    for(const auto& i : ctx.structs) {
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
    for(const auto& i : ctx.functions) {
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

}