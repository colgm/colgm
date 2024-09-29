#include "mir/ast2mir.h"

namespace colgm::mir {

bool ast2mir::visit_unary_operator(ast::unary_operator* node) {
    auto new_block = new mir_block(node->get_value()->get_location());
    auto temp = block;
    block = new_block;
    node->get_value()->accept(this);
    block = temp;

    mir_unary::opr_kind type = mir_unary::opr_kind::neg;
    switch(node->get_opr()) {
        case ast::unary_operator::kind::neg: type = mir_unary::opr_kind::neg; break;
        case ast::unary_operator::kind::bnot: type = mir_unary::opr_kind::bnot; break;
        case ast::unary_operator::kind::lnot: type = mir_unary::opr_kind::lnot; break;
    }

    block->add_content(new mir_unary(
        node->get_location(),
        type,
        node->get_resolve(),
        new_block
    ));
    return true;
}

bool ast2mir::visit_binary_operator(ast::binary_operator* node) {
    auto left = new mir_block(node->get_left()->get_location());
    auto temp = block;
    block = left;
    node->get_left()->accept(this);

    auto right = new mir_block(node->get_right()->get_location());
    block = right;
    node->get_right()->accept(this);
    block = temp;

    mir_binary::opr_kind type = mir_binary::opr_kind::add;
    switch(node->get_opr()) {
        case ast::binary_operator::kind::add: type = mir_binary::opr_kind::add; break;
        case ast::binary_operator::kind::sub: type = mir_binary::opr_kind::sub; break;
        case ast::binary_operator::kind::mult: type = mir_binary::opr_kind::mult; break;
        case ast::binary_operator::kind::div: type = mir_binary::opr_kind::div; break;
        case ast::binary_operator::kind::rem: type = mir_binary::opr_kind::rem; break;
        case ast::binary_operator::kind::cmpeq: type = mir_binary::opr_kind::cmpeq; break;
        case ast::binary_operator::kind::cmpneq: type = mir_binary::opr_kind::cmpneq; break;
        case ast::binary_operator::kind::less: type = mir_binary::opr_kind::less; break;
        case ast::binary_operator::kind::leq: type = mir_binary::opr_kind::leq; break;
        case ast::binary_operator::kind::grt: type = mir_binary::opr_kind::grt; break;
        case ast::binary_operator::kind::geq: type = mir_binary::opr_kind::geq; break;
        case ast::binary_operator::kind::cmpand: type = mir_binary::opr_kind::cmpand; break;
        case ast::binary_operator::kind::cmpor: type = mir_binary::opr_kind::cmpor; break;
        case ast::binary_operator::kind::band: type = mir_binary::opr_kind::band; break;
        case ast::binary_operator::kind::bor: type = mir_binary::opr_kind::bor; break;
        case ast::binary_operator::kind::bxor: type = mir_binary::opr_kind::bxor; break;
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

bool ast2mir::visit_type_convert(ast::type_convert* node) {
    auto new_block = new mir_block(node->get_source()->get_location());
    auto temp = block;
    block = new_block;
    node->get_source()->accept(this);
    block = temp;

    auto target_type = generate_type(node->get_target());
    if (ctx.global_symbol.count(target_type.name) &&
        ctx.global_symbol.at(target_type.name).kind == sym_kind::enum_kind) {
        target_type.is_enum = true;
    }
    block->add_content(new mir_type_convert(
        node->get_location(),
        new_block,
        target_type
    ));
    return true;
}

bool ast2mir::visit_nil_literal(ast::nil_literal* node) {
    block->add_content(new mir_nil(
        node->get_location(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_number_literal(ast::number_literal* node) {
    block->add_content(new mir_number(
        node->get_location(),
        node->get_number(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_string_literal(ast::string_literal* node) {
    block->add_content(new mir_string(
        node->get_location(),
        node->get_string(),
        node->get_resolve()
    ));
    mctx.const_strings.insert({node->get_string(), mctx.const_strings.size()});
    return true;
}

bool ast2mir::visit_char_literal(ast::char_literal* node) {
    block->add_content(new mir_char(
        node->get_location(),
        node->get_char(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_bool_literal(ast::bool_literal* node) {
    block->add_content(new mir_bool(
        node->get_location(),
        node->get_flag(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_array_literal(ast::array_literal* node) {
    const auto& num_str = node->get_size()->get_number();
    u64 res = (num_str.size()>=2 && num_str[0]=='0' && num_str[1]=='x')
              ? hex_to_u64(num_str.c_str())
              : dec_to_u64(num_str.c_str());
    block->add_content(new mir_array(
        node->get_location(),
        res,
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_struct_decl(ast::struct_decl* node) {
    auto new_struct = new mir_struct;
    new_struct->name = node->get_name();
    new_struct->location = node->get_location();

    for(auto i : node->get_fields()) {
        new_struct->field_type.push_back(generate_type(i->get_type()));
    }
    mctx.structs.push_back(new_struct);
    return true;
}

bool ast2mir::visit_func_decl(ast::func_decl* node) {
    auto name = node->get_name();
    if (impl_struct_name.length()) {
        const auto tmp = type {
            .name = impl_struct_name,
            .loc_file = node->get_location().file
        };
        name = mangle(tmp.full_path_name()) + "." + name;
    }
    // global function which is not extern
    if (impl_struct_name.empty() && !node->is_extern_func()) {
        const auto tmp = type {
            .name = name,
            .loc_file = node->get_location().file
        };
        name = mangle(tmp.full_path_name());
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
        mctx.decls.push_back(func);
        return true;
    }

    block = func->block = new mir_block(node->get_code_block()->get_location());
    for(auto i : node->get_code_block()->get_stmts()) {
        i->accept(this);
        if (i->get_ast_type()==ast::ast_type::ast_ret_stmt ||
            i->get_ast_type()==ast::ast_type::ast_break_stmt ||
            i->get_ast_type()==ast::ast_type::ast_continue_stmt) {
            break;
        }
    }
    block = nullptr;
    mctx.impls.push_back(func);
    return true;
}

bool ast2mir::visit_impl_struct(ast::impl_struct* node) {
    impl_struct_name = node->get_struct_name();
    for(auto i : node->get_methods()) {
        i->accept(this);
    }
    impl_struct_name = "";
    return true;
}

bool ast2mir::visit_call_index(ast::call_index* node) {
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

bool ast2mir::visit_call_func_args(ast::call_func_args* node) {
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

bool ast2mir::visit_get_field(ast::get_field* node) {
    block->add_content(new mir_get_field(
        node->get_location(),
        node->get_name(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_ptr_get_field(ast::ptr_get_field* node) {
    block->add_content(new mir_ptr_get_field(
        node->get_location(),
        node->get_name(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_initializer(ast::initializer* node) {
    auto temp = block;
    auto struct_init = new mir_struct_init(
        node->get_location(),
        node->get_resolve()
    );
    for(auto i : node->get_pairs()) {
        block = new mir_block(i->get_location());
        i->get_value()->accept(this);
        struct_init->add_field(
            i->get_field()->get_name(),
            block,
            i->get_value()->get_resolve()
        );
    }
    block = temp;
    block->add_content(struct_init);
    return true;
}

bool ast2mir::visit_call_path(ast::call_path* node) {
    block->add_content(new mir_get_path(
        node->get_location(),
        node->get_name(),
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_call(ast::call* node) {
    auto new_block = new mir_block(node->get_location());
    auto temp = block;
    block = new_block;
    block->add_content(new mir_call_id(
        node->get_head()->get_location(),
        node->get_head()->get_id()->get_name(),
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

bool ast2mir::visit_definition(ast::definition* node) {
    auto new_block = new mir_block(node->get_init_value()->get_location());
    auto temp = block;
    block = new_block;
    node->get_init_value()->accept(this);
    block = temp;

    block->add_content(new mir_define(
        node->get_location(),
        node->get_name(),
        new_block,
        node->get_resolve()
    ));
    return true;
}

bool ast2mir::visit_assignment(ast::assignment* node) {
    auto left = new mir_block(node->get_left()->get_location());
    auto temp = block;
    block = left;
    node->get_left()->accept(this);

    auto right = new mir_block(node->get_right()->get_location());
    block = right;
    node->get_right()->accept(this);
    block = temp;

    mir_assign::opr_kind type = mir_assign::opr_kind::eq;
    switch(node->get_type()) {
        case ast::assignment::kind::eq: type = mir_assign::opr_kind::eq; break;
        case ast::assignment::kind::addeq: type = mir_assign::opr_kind::addeq; break;
        case ast::assignment::kind::subeq: type = mir_assign::opr_kind::subeq; break;
        case ast::assignment::kind::multeq: type = mir_assign::opr_kind::multeq; break;
        case ast::assignment::kind::diveq: type = mir_assign::opr_kind::diveq; break;
        case ast::assignment::kind::remeq: type = mir_assign::opr_kind::remeq; break;
        case ast::assignment::kind::andeq: type = mir_assign::opr_kind::andeq; break;
        case ast::assignment::kind::xoreq: type = mir_assign::opr_kind::xoreq; break;
        case ast::assignment::kind::oreq: type = mir_assign::opr_kind::oreq; break;
    }
    block->add_content(new mir_assign(node->get_location(), type, left, right));
    return true;
}

bool ast2mir::visit_cond_stmt(ast::cond_stmt* node) {
    auto cond = new mir_branch(node->get_location());
    for(auto i : node->get_stmts()) {
        cond->add(generate_if_stmt(i));
    }
    block->add_content(cond);
    return true;
}

mir_if* ast2mir::generate_if_stmt(ast::if_stmt* node) {
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

bool ast2mir::visit_match_stmt(ast::match_stmt* node) {
    auto new_block = new mir_block(node->get_value()->get_location());
    auto temp = block;
    block = new_block;
    node->get_value()->accept(this);
    block = temp;

    const auto name = "match." + std::to_string(reinterpret_cast<u64>(node->get_value()));

    block->add_content(new mir_define(
        node->get_value()->get_location(),
        name,
        new_block,
        node->get_value()->get_resolve()
    ));

    auto cond = new mir_branch(node->get_location());
    for(auto i : node->get_cases()) {
        cond->add(generate_match_case(i, name));
    }
    block->add_content(cond);
    return true;
}

mir_if* ast2mir::generate_match_case(ast::match_case* node,
                                     const std::string& tmp_variable) {
    auto temp = block;

    // generate match._xxx mir code
    auto left_call_block = new mir_block(node->get_value()->get_location());
    left_call_block->add_content(new mir_call_id(
        node->get_value()->get_location(),
        tmp_variable,
        node->get_value()->get_resolve()
    ));
    auto left_block = new mir_block(node->get_value()->get_location());
    left_block->add_content(new mir_call(
        node->get_value()->get_location(),
        node->get_value()->get_resolve(),
        left_call_block
    ));

    auto right_block = new mir_block(node->get_value()->get_location());
    block = right_block;
    node->get_value()->accept(this);

    auto cond_block = new mir_block(node->get_location());
    cond_block->add_content(new mir_binary(
        node->get_location(),
        mir_binary::opr_kind::cmpeq,
        type::bool_type(),
        left_block,
        right_block
    ));

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

bool ast2mir::visit_while_stmt(ast::while_stmt* node) {
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

    block->add_content(new mir_loop(
        node->get_location(),
        cond_block,
        body_block,
        nullptr // no update in while loop
    ));
    return true;
}

bool ast2mir::visit_for_stmt(ast::for_stmt* node) {
    auto temp = block;

    // for top level block will keep defined variable in live range
    // util for loop exit
    auto for_top_level = new mir_block(node->get_location());
    block->add_content(for_top_level);

    if (node->get_init()) {
        block = for_top_level;
        node->get_init()->accept(this);
        block = temp;
    }

    auto cond_block = new mir_block(node->get_condition()
                                    ? node->get_condition()->get_location()
                                    : node->get_location());
    if (node->get_condition()) {
        block = cond_block;
        node->get_condition()->accept(this);
        block = temp;
    } else {
        auto cond = new mir_binary(
            node->get_location(),
            mir_binary::opr_kind::cmpeq,
            type::bool_type(),
            new mir_block(node->get_location()),
            new mir_block(node->get_location())
        );
        cond->get_left()->add_content(new mir_number(
            node->get_location(),
            "1",
            type::i64_type()
        ));
        cond->get_right()->add_content(new mir_number(
            node->get_location(),
            "1",
            type::i64_type()
        ));
        cond_block->add_content(cond);
    }

    auto update_block = new mir_block(node->get_update()
                                    ? node->get_update()->get_location()
                                    : node->get_location());
    if (node->get_update()) {
        block = update_block;
        node->get_update()->accept(this);
        block = temp;
    }

    auto body_block = new mir_block(node->get_block()->get_location());
    block = body_block;
    for(auto i : node->get_block()->get_stmts()) {
        i->accept(this);
    }
    block = temp;

    for_top_level->add_content(new mir_loop(
        node->get_location(),
        cond_block,
        body_block,
        update_block
    ));
    return true;
}

bool ast2mir::visit_ret_stmt(ast::ret_stmt* node) {
    auto value_block = new mir_block(node->get_value()?
        node->get_value()->get_location():node->get_location());
    auto temp = block;
    block = value_block;
    if (node->get_value()) {
        node->get_value()->accept(this);
    }
    block = temp;

    block->add_content(new mir_return(
        node->get_location(),
        value_block
    ));
    return true;
}

bool ast2mir::visit_continue_stmt(ast::continue_stmt* node) {
    block->add_content(new mir_continue(node->get_location()));
    return true;
}

bool ast2mir::visit_break_stmt(ast::break_stmt* node) {
    block->add_content(new mir_break(node->get_location()));
    return true;
}

bool ast2mir::visit_code_block(ast::code_block* node) {
    auto new_block = new mir_block(node->get_location());
    block->add_content(new_block);

    auto temp = block;
    block = new_block;
    for(auto i : node->get_stmts()) {
        i->accept(this);
        if (i->get_ast_type()==ast::ast_type::ast_ret_stmt ||
            i->get_ast_type()==ast::ast_type::ast_break_stmt ||
            i->get_ast_type()==ast::ast_type::ast_continue_stmt) {
            break;
        }
    }
    block = temp;
    return true;
}

type ast2mir::generate_type(ast::type_def* node) {
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

void ast2mir::dump(std::ostream& os) {
    std::vector<std::string> const_strings;
    const_strings.resize(mctx.const_strings.size());
    for(const auto& i : mctx.const_strings) {
        const_strings[i.second] = i.first;
    }
    for(usize i = 0; i<mctx.const_strings.size(); ++i) {
        os << i << " \"" << llvm_raw_string(const_strings[i]) << "\"\n";
    }
    if (const_strings.size()) {
        os << "\n";
    }

    for(const auto i : mctx.structs) {
        os << i->name << " {";
        size_t count = 0;
        for(const auto& f : i->field_type) {
            os << f;
            ++count;
            if (count!=i->field_type.size()) {
                os << ", ";
            }
        }
        os << "} [" << i->location << "]\n";
    }
    if (mctx.structs.size()) {
        os << "\n";
    }

    for(const auto i : mctx.decls) {
        os << i->name << "(";
        for(const auto& p : i->params) {
            os << p.first << ": " << p.second;
            if (p.first!=i->params.back().first) {
                os << ", ";
            }
        }
        os << ") -> " << i->return_type << "\n";
    }
    if (mctx.decls.size()) {
        os << "\n";
    }

    for(const auto i : mctx.impls) {
        os << i->name << "(";
        for(const auto& p : i->params) {
            os << p.first << ": " << p.second;
            if (p.first!=i->params.back().first) {
                os << ", ";
            }
        }
        os << ") -> " << i->return_type;
        if (i->block) {
            os << " {\n";
            for(auto ir : i->block->get_content()) {
                ir->dump(" ", os);
            }
            os << "}\n";
        }
        os << "\n";
    }
}

}