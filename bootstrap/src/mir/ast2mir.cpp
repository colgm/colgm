#include "mir/ast2mir.h"

namespace colgm::mir {

bool ast2mir::visit_unary_operator(unary_operator* node) {
    auto new_block = new mir_block(node->get_value()->get_location());
    auto temp = block;
    block = new_block;
    node->get_value()->accept(this);
    block = temp;

    mir_unary::opr_kind type;
    switch(node->get_opr()) {
        case unary_operator::kind::bnot: type = mir_unary::opr_kind::bnot; break;
        case unary_operator::kind::neg: type = mir_unary::opr_kind::neg; break;
    }

    block->add_content(new mir_unary(
        node->get_location(),
        type,
        node->get_resolve(),
        new_block
    ));
    return true;
}

bool ast2mir::visit_binary_operator(binary_operator* node) {
    auto left = new mir_block(node->get_left()->get_location());
    auto temp = block;
    block = left;
    node->get_left()->accept(this);

    auto right = new mir_block(node->get_right()->get_location());
    block = right;
    node->get_right()->accept(this);
    block = temp;

    mir_binary::opr_kind type;
    switch(node->get_opr()) {
        case binary_operator::kind::add: type = mir_binary::opr_kind::add; break;
        case binary_operator::kind::sub: type = mir_binary::opr_kind::sub; break;
        case binary_operator::kind::mult: type = mir_binary::opr_kind::mult; break;
        case binary_operator::kind::div: type = mir_binary::opr_kind::div; break;
        case binary_operator::kind::rem: type = mir_binary::opr_kind::rem; break;
        case binary_operator::kind::cmpeq: type = mir_binary::opr_kind::cmpeq; break;
        case binary_operator::kind::cmpneq: type = mir_binary::opr_kind::cmpneq; break;
        case binary_operator::kind::less: type = mir_binary::opr_kind::less; break;
        case binary_operator::kind::leq: type = mir_binary::opr_kind::leq; break;
        case binary_operator::kind::grt: type = mir_binary::opr_kind::grt; break;
        case binary_operator::kind::geq: type = mir_binary::opr_kind::geq; break;
        case binary_operator::kind::cmpand: type = mir_binary::opr_kind::cmpand; break;
        case binary_operator::kind::cmpor: type = mir_binary::opr_kind::cmpor; break;
        case binary_operator::kind::band: type = mir_binary::opr_kind::band; break;
        case binary_operator::kind::bor: type = mir_binary::opr_kind::bor; break;
        case binary_operator::kind::bxor: type = mir_binary::opr_kind::bxor; break;
    }
    block->add_content(new mir_binary(
        node->get_location(),
        type,
        node->get_resolve(),
        left,
        right
    ));
    return true;
}

bool ast2mir::visit_type_convert(type_convert* node) {
    auto new_block = new mir_block(node->get_source()->get_location());
    auto temp = block;
    block = new_block;
    node->get_source()->accept(this);
    block = temp;

    block->add_content(new mir_type_convert(
        node->get_location(),
        new_block,
        generate_type(node->get_target())
    ));
    return true;
}

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
    for(auto i : node->get_code_block()->get_stmts()) {
        i->accept(this);
    }
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

bool ast2mir::visit_call_index(call_index* node) {
    auto new_block = new mir_block(node->get_index()->get_location());
    auto temp = block;
    block = new_block;
    node->get_index()->accept(this);
    block = temp;

    block->add_content(new mir_call_index(
        node->get_location(),
        node->get_resolve(),
        new_block
    ));
    return true;
}

bool ast2mir::visit_call_func_args(call_func_args* node) {
    auto args_block = new mir_block(node->get_location());
    auto temp = block;
    block = args_block;
    for(auto i : node->get_args()) {
        i->accept(this);
    }
    block = temp;

    block->add_content(new mir_call_func(
        node->get_location(),
        args_block,
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_call_field(call_field* node) {
    block->add_content(new mir_call_field(
        node->get_location(),
        node->get_name(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_ptr_call_field(ptr_call_field* node) {
    block->add_content(new mir_ptr_call_field(
        node->get_location(),
        node->get_name(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_call_path(call_path* node) {
    block->add_content(new mir_call_path(
        node->get_location(),
        node->get_name(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_call(call* node) {
    auto new_block = new mir_block(node->get_location());
    auto temp = block;
    block = new_block;
    block->add_content(new mir_call_id(
        node->get_head()->get_location(),
        node->get_head()->get_name(),
        node->get_head()->get_resolve()
    ));
    for(auto i : node->get_chain()) {
        i->accept(this);
    }
    block = temp;

    block->add_content(new mir_call(
        node->get_location(),
        node->get_resolve(),
        new_block
    ));
    return true;
}

bool ast2mir::visit_definition(definition* node) {
    auto new_block = new mir_block(node->get_init_value()->get_location());
    auto temp = block;
    block = new_block;
    node->get_init_value()->accept(this);
    block = temp;

    block->add_content(new mir_define(
        node->get_location(),
        node->get_name(),
        new_block,
        node->get_type()? generate_type(node->get_type()):node->get_resolve(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_assignment(assignment* node) {
    auto left = new mir_block(node->get_left()->get_location());
    auto temp = block;
    block = left;
    node->get_left()->accept(this);

    auto right = new mir_block(node->get_right()->get_location());
    block = right;
    node->get_right()->accept(this);
    block = temp;

    mir_assign::opr_kind type;
    switch(node->get_type()) {
        case assignment::kind::eq: type = mir_assign::opr_kind::eq; break;
        case assignment::kind::addeq: type = mir_assign::opr_kind::addeq; break;
        case assignment::kind::subeq: type = mir_assign::opr_kind::subeq; break;
        case assignment::kind::multeq: type = mir_assign::opr_kind::multeq; break;
        case assignment::kind::diveq: type = mir_assign::opr_kind::diveq; break;
        case assignment::kind::remeq: type = mir_assign::opr_kind::remeq; break;
        case assignment::kind::andeq: type = mir_assign::opr_kind::andeq; break;
        case assignment::kind::xoreq: type = mir_assign::opr_kind::xoreq; break;
        case assignment::kind::oreq: type = mir_assign::opr_kind::oreq; break;
    }
    block->add_content(new mir_assign(
        node->get_location(),
        mir_assign::opr_kind::addeq,
        left,
        right
    ));
    return true;
}

bool ast2mir::visit_cond_stmt(cond_stmt* node) {
    auto cond = new mir_branch(node->get_location());
    for(auto i : node->get_stmts()) {
        cond->add(generate_if_stmt(i));
    }
    block->add_content(cond);
    return true;
}

mir_if* ast2mir::generate_if_stmt(if_stmt* node) {
    // else branch
    if (!node->get_condition()) {
        auto body_block = new mir_block(node->get_block()->get_location());
        auto temp = block;
        block = body_block;
        for(auto i : node->get_block()->get_stmts()) {
            i->accept(this);
        }
        block = temp;

        return new mir_if(
            node->get_location(),
            nullptr,
            body_block
        );
    }
    auto cond_block = new mir_block(node->get_condition()->get_location());
    auto temp = block;
    block = cond_block;
    node->get_condition()->accept(this);

    auto body_block = new mir_block(node->get_block()->get_location());
    block = body_block;
    for(auto i : node->get_block()->get_stmts()) {
        i->accept(this);
    }
    block = temp;

    return new mir_if(
        node->get_location(),
        cond_block,
        body_block
    );
}

bool ast2mir::visit_while_stmt(while_stmt* node) {
    auto cond_block = new mir_block(node->get_condition()->get_location());
    auto temp = block;
    block = cond_block;
    node->get_condition()->accept(this);
    auto body_block = new mir_block(node->get_block()->get_location());
    block = body_block;
    for(auto i : node->get_block()->get_stmts()) {
        i->accept(this);
    }
    block = temp;

    block->add_content(new mir_while(
        node->get_location(),
        cond_block,
        body_block
    ));
    return true;
}

bool ast2mir::visit_continue_stmt(continue_stmt* node) {
    block->add_content(new mir_continue(node->get_location()));
    return true;
}

bool ast2mir::visit_break_stmt(break_stmt* node) {
    block->add_content(new mir_break(node->get_location()));
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
    if (mctx.impls.count(f->name)) {
        err.err("ast2mir",
            f->location,
            "redefinition of function \"" + f->name + "\"."
        );
    }
    mctx.impls.insert({f->name, f});
}

void ast2mir::dump(std::ostream& os) {
    for(const auto& i : mctx.impls) {
        os << i.first << "(";
        for(const auto& p : i.second->params) {
            os << p.first << ": " << p.second;
            if (p.first!=i.second->params.back().first) {
                os << ", ";
            }
        }
        os << ") -> " << i.second->return_type;
        if (i.second->block) {
            os << " {\n";
            for(auto ir : i.second->block->get_content()) {
                ir->dump("  ", os);
            }
            os << "}";
        }
        os << "\n\n";
    }
}

}