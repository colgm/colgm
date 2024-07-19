#include "mir/ast2mir.h"

namespace colgm::mir {

bool ast2mir::visit_nil_literal(nil_literal* node) {
    block->add_content(new mir_nil(
        node->get_location(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_number_literal(number_literal* node) {
    block->add_content(new mir_number(
        node->get_location(),
        node->get_number(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_string_literal(string_literal* node) {
    block->add_content(new mir_string(
        node->get_location(),
        node->get_string(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_char_literal(char_literal* node) {
    block->add_content(new mir_char(
        node->get_location(),
        node->get_char(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_bool_literal(bool_literal* node) {
    block->add_content(new mir_bool(
        node->get_location(),
        node->get_flag(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_func_decl(func_decl* node) {
    auto name = node->get_name();
    if (impl_struct_name.length()) {
        const auto tmp = type {
            .name = impl_struct_name,
            .loc_file = node->get_location().file
        };
        name = mangle_in_module_symbol(tmp.full_path_name()) + "." + name;
    }
    name = "@" + name;

    auto func = new mir_func();
    func->name = name;
    func->location = node->get_location();
    func->return_type = generate_type(node->get_return_type());
    for(auto i : node->get_params()->get_params()) {
        func->params.push_back({
            i->get_name()->get_name(),
            generate_type(i->get_type())
        });
    }

    // insert if is declaration
    if (!node->get_code_block()) {
        emit_function(func);
        return true;
    }

    block = func->block = new mir_block(node->get_code_block()->get_location());
    node->get_code_block()->accept(this);
    block = nullptr;
    emit_function(func);
    return true;
}

bool ast2mir::visit_impl_struct(impl_struct* node) {
    impl_struct_name = node->get_struct_name();
    for(auto i : node->get_methods()) {
        i->accept(this);
    }
    impl_struct_name = "";
    return true;
}

bool ast2mir::visit_code_block(code_block* node) {
    auto new_block = new mir_block(node->get_location());
    block->add_content(new_block);

    auto temp = block;
    block = new_block;
    for(auto i : node->get_stmts()) {
        i->accept(this);
    }
    block = temp;
    return true;
}

type ast2mir::generate_type(type_def* node) {
    const auto& name = node->get_name()->get_name();
    auto loc_file = node->get_location().file;
    if (ctx.global_symbol.count(name)) {
        loc_file = ctx.global_symbol.at(name).loc_file;
    }
    return type {
        .name = name,
        .loc_file = loc_file,
        .pointer_depth = node->get_pointer_level()
    };
}

void ast2mir::emit_function(mir_func* f) {
    if (impls.count(f->name)) {
        err.err("ast2mir",
            f->location,
            "redefinition of function \"" + f->name + "\"."
        );
    }
    impls.insert({f->name, f});
}

void ast2mir::dump() const {
    for(const auto& i : impls) {
        std::cout << i.first << "(";
        for(const auto& p : i.second->params) {
            std::cout << p.first << ": " << p.second;
            if (p.first!=i.second->params.back().first) {
                std::cout << ", ";
            }
        }
        std::cout << ") -> " << i.second->return_type;
        if (i.second->block) {
            std::cout << " {\n";
            i.second->block->dump("  ", std::cout);
            std::cout << "}";
        }
        std::cout << "\n";
    }
}

}