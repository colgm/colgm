#include "parse.h"

namespace colgm {

void parse::match(tok type) {
    if (type==toks[ptr].type) {
        next();
        return;
    }
    switch(type) {
        case tok::tk_id:
            err.err("parse",
                toks[ptr].loc,
                "expected identifier but get \"" + toks[ptr].str + "\"."
            );
            break;
        case tok::tk_num:
            err.err("parse",
                toks[ptr].loc,
                "expected number but get \"" + toks[ptr].str + "\"."
            );
            break;
        case tok::tk_str:
            err.err("parse",
                toks[ptr].loc,
                "expected string but get \"" + toks[ptr].str + "\"."
            );
            break;
        default:
            err.err("parse",
                toks[ptr].loc,
                "expected \"" + tokname.at(type) +
                "\" but get \"" + toks[ptr].str + "\"."
            );
            break;
    }
    // should be panic mode here, but this is a bootstrap
    // so do not do too many things
    next();
    return;
}

bool parse::look_ahead(tok type) {
    return type==toks[ptr].type;
}

void parse::update_location(node* n) {
    n->update_location(toks[ptr-1].loc);
}

identifier* parse::identifier_gen() {
    auto result = new identifier(toks[ptr].loc, toks[ptr].str);
    match(tok::tk_id);
    return result;
}

call* parse::call_gen() {
    auto result = new call(toks[ptr].loc);
    result->set_head(identifier_gen());
    while(look_ahead(tok::tk_lcurve) || look_ahead(tok::tk_lbracket) ||
          look_ahead(tok::tk_dot) || look_ahead(tok::tk_arrow) ||
          look_ahead(tok::tk_double_colon) || look_ahead(tok::tk_lbrace)) {
        if (look_ahead(tok::tk_lcurve)) {
            match(tok::tk_lcurve);
            auto new_call_func = new call_func_args(toks[ptr].loc);
            while(!look_ahead(tok::tk_rcurve)) {
                new_call_func->add_arg(calculation_gen());
                if (look_ahead(tok::tk_comma)) {
                    match(tok::tk_comma);
                } else {
                    break;
                }
            }
            match(tok::tk_rcurve);
            update_location(new_call_func);
            result->add_chain(new_call_func);
        } else if (look_ahead(tok::tk_lbracket)) {
            match(tok::tk_lbracket);
            auto new_call_index = new call_index(toks[ptr].loc);
            new_call_index->set_index(calculation_gen());
            match(tok::tk_rbracket);
            update_location(new_call_index);
            result->add_chain(new_call_index);
        } else if (look_ahead(tok::tk_dot)) {
            match(tok::tk_dot);
            auto new_call_field = new get_field(toks[ptr].loc, toks[ptr].str);
            match(tok::tk_id);
            update_location(new_call_field);
            result->add_chain(new_call_field);
        } else if (look_ahead(tok::tk_arrow)) {
            match(tok::tk_arrow);
            auto new_call_field = new ptr_get_field(toks[ptr].loc, toks[ptr].str);
            match(tok::tk_id);
            update_location(new_call_field);
            result->add_chain(new_call_field);
        } else if (look_ahead(tok::tk_double_colon)) {
            match(tok::tk_double_colon);
            auto new_call_path = new call_path(toks[ptr].loc, toks[ptr].str);
            match(tok::tk_id);
            update_location(new_call_path);
            result->add_chain(new_call_path);
        } else if (look_ahead(tok::tk_lbrace)) {
            result->add_chain(initializer_gen());
        }
    }
    update_location(result);
    return result;
}

initializer* parse::initializer_gen() {
    auto result = new initializer(toks[ptr].loc);

    match(tok::tk_lbrace);
    while(!look_ahead(tok::tk_rbrace)) {
        auto pair_node = new init_pair(toks[ptr].loc);

        pair_node->set_field(identifier_gen());
        match(tok::tk_colon);
        pair_node->set_value(calculation_gen());
        update_location(pair_node);

        result->add_pair(pair_node);
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else {
            break;
        }
    }
    match(tok::tk_rbrace);

    update_location(result);
    return result;
}

nil_literal* parse::nil_gen() {
    auto result = new nil_literal(toks[ptr].loc);
    match(tok::tk_nil);
    return result;
}

number_literal* parse::number_gen() {
    auto result = new number_literal(toks[ptr].loc, toks[ptr].str);
    match(tok::tk_num);
    return result;
}

string_literal* parse::string_gen() {
    auto result = new string_literal(toks[ptr].loc, toks[ptr].str);
    match(tok::tk_str);
    return result;
}

char_literal* parse::char_gen() {
    auto result = new char_literal(toks[ptr].loc, toks[ptr].str[0]);
    match(tok::tk_ch);
    return result;
}

bool_literal* parse::bool_gen() {
    auto result = new bool_literal(toks[ptr].loc, toks[ptr].type==tok::tk_true);
    match(toks[ptr].type);
    return result;
}

array_literal* parse::array_gen() {
    auto result = new array_literal(toks[ptr].loc);
    match(tok::tk_lbracket);
    result->set_type(type_gen());
    match(tok::tk_semi);
    result->set_size(number_gen());
    match(tok::tk_rbracket);
    update_location(result);
    return result;
}

expr* parse::scalar_gen() {
    if (look_ahead(tok::tk_lcurve)) {
        match(tok::tk_lcurve);
        auto result = calculation_gen();
        match(tok::tk_rcurve);
        return result;
    }
    if (look_ahead(tok::tk_sub)) {
        return unary_neg_gen();
    } else if (look_ahead(tok::tk_floater)) {
        return unary_bnot_gen();
    } else if (look_ahead(tok::tk_nil)) {
        return nil_gen();
    } else if (look_ahead(tok::tk_num)) {
        return number_gen();
    } else if (look_ahead(tok::tk_str)) {
        return string_gen();
    } else if (look_ahead(tok::tk_ch)) {
        return char_gen();
    } else if (look_ahead(tok::tk_true) || look_ahead(tok::tk_false)) {
        return bool_gen();
    } else if (look_ahead(tok::tk_lbracket)) {
        return array_gen();
    } else if (look_ahead(tok::tk_id)) {
        return call_gen();
    }
    err.err("parse", toks[ptr].loc, "expected scalar here.");
    next();
    return new null();
}

unary_operator* parse::unary_neg_gen() {
    auto result = new unary_operator(toks[ptr].loc);
    match(tok::tk_sub);
    result->set_value(scalar_gen());
    result->set_opr(unary_operator::kind::neg);
    update_location(result);
    return result;
}

unary_operator* parse::unary_bnot_gen() {
    auto result = new unary_operator(toks[ptr].loc);
    match(tok::tk_floater);
    result->set_value(scalar_gen());
    result->set_opr(unary_operator::kind::bnot);
    update_location(result);
    return result;
}

expr* parse::type_convert_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = scalar_gen();
    if (look_ahead(tok::tk_wide_arrow)) {
        match(tok::tk_wide_arrow);
        auto type_cast_node = new type_convert(begin_location);
        type_cast_node->set_source(result);
        type_cast_node->set_target(type_gen());
        result = type_cast_node;
    }

    update_location(result);
    return result;
}

expr* parse::multive_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = type_convert_gen();
    while(look_ahead(tok::tk_mult) ||
        look_ahead(tok::tk_div) ||
        look_ahead(tok::tk_rem)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(result);
        switch(toks[ptr].type) {
            case tok::tk_mult: binary->set_opr(binary_operator::kind::mult); break;
            case tok::tk_div: binary->set_opr(binary_operator::kind::div); break;
            case tok::tk_rem: binary->set_opr(binary_operator::kind::rem); break;
            default: break;
        }
        match(toks[ptr].type);
        binary->set_right(type_convert_gen());
        result = binary;
    }
    update_location(result);
    return result;
}

expr* parse::additive_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = multive_gen();
    while(look_ahead(tok::tk_add) || look_ahead(tok::tk_sub)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(result);
        switch(toks[ptr].type) {
            case tok::tk_add: binary->set_opr(binary_operator::kind::add); break;
            case tok::tk_sub: binary->set_opr(binary_operator::kind::sub); break;
            default: break;
        }
        match(toks[ptr].type);
        binary->set_right(multive_gen());
        result = binary;
    }
    update_location(result);
    return result;
}

expr* parse::bitwise_and_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = additive_gen();
    while(look_ahead(tok::tk_btand)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(result);
        binary->set_opr(binary_operator::kind::band);
        match(toks[ptr].type);
        binary->set_right(additive_gen());
        result = binary;
    }
    update_location(result);
    return result;
}

expr* parse::bitwise_xor_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = bitwise_and_gen();
    while(look_ahead(tok::tk_btxor)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(result);
        binary->set_opr(binary_operator::kind::bxor);
        match(toks[ptr].type);
        binary->set_right(bitwise_and_gen());
        result = binary;
    }
    update_location(result);
    return result;
}

expr* parse::bitwise_or_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = bitwise_xor_gen();
    while(look_ahead(tok::tk_btor)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(result);
        binary->set_opr(binary_operator::kind::bor);
        match(toks[ptr].type);
        binary->set_right(bitwise_xor_gen());
        result = binary;
    }
    update_location(result);
    return result;
}

expr* parse::compare_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = bitwise_or_gen();
    while(look_ahead(tok::tk_cmpeq) || look_ahead(tok::tk_neq) ||
        look_ahead(tok::tk_less) || look_ahead(tok::tk_leq) ||
        look_ahead(tok::tk_grt) || look_ahead(tok::tk_geq)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(result);
        switch(toks[ptr].type) {
            case tok::tk_cmpeq: binary->set_opr(binary_operator::kind::cmpeq); break;
            case tok::tk_neq: binary->set_opr(binary_operator::kind::cmpneq); break;
            case tok::tk_less: binary->set_opr(binary_operator::kind::less); break;
            case tok::tk_leq: binary->set_opr(binary_operator::kind::leq); break;
            case tok::tk_grt: binary->set_opr(binary_operator::kind::grt); break;
            case tok::tk_geq: binary->set_opr(binary_operator::kind::geq); break;
            default: break;
        }
        match(toks[ptr].type);
        binary->set_right(bitwise_or_gen());
        result = binary;
    }
    update_location(result);
    return result;
}

expr* parse::not_expression_gen() {
    const auto begin_location = toks[ptr].loc;
    if (!look_ahead(tok::tk_opnot)) {
        return compare_gen();
    }
    match(tok::tk_opnot);
    auto value = compare_gen();
    auto result = new unary_operator(begin_location);
    result->set_opr(unary_operator::kind::lnot);
    result->set_value(value);
    update_location(result);
    return result;
}

expr* parse::and_expression_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = not_expression_gen();
    if (!look_ahead(tok::tk_opand)) {
        return result;
    }

    auto binary = new binary_operator(begin_location);
    binary->set_left(result);
    binary->set_opr(binary_operator::kind::cmpand);
    match(tok::tk_opand);
    binary->set_right(and_expression_gen());
    result = binary;
    update_location(result);
    return result;
}

expr* parse::or_expression_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = and_expression_gen();
    if (!look_ahead(tok::tk_opor)) {
        return result;
    }

    auto binary = new binary_operator(begin_location);
    binary->set_left(result);
    binary->set_opr(binary_operator::kind::cmpor);
    match(tok::tk_opor);
    binary->set_right(or_expression_gen());
    result = binary;
    update_location(result);
    return result;
}

expr* parse::calculation_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = or_expression_gen();
    if (!result) {
        return result;
    }
    if (result->get_ast_type()!=ast_type::ast_call) {
        return result;
    }
    // if is call node, check assignment syntax
    if (look_ahead(tok::tk_addeq) || look_ahead(tok::tk_subeq) ||
        look_ahead(tok::tk_multeq) || look_ahead(tok::tk_diveq) ||
        look_ahead(tok::tk_remeq) || look_ahead(tok::tk_eq) ||
        look_ahead(tok::tk_btandeq) || look_ahead(tok::tk_btxoreq) ||
        look_ahead(tok::tk_btoreq)) {
        auto new_assignment = new assignment(begin_location);
        new_assignment->set_left(reinterpret_cast<call*>(result));
        switch(toks[ptr].type) {
            case tok::tk_eq: new_assignment->set_type(assignment::kind::eq); break;
            case tok::tk_addeq: new_assignment->set_type(assignment::kind::addeq); break;
            case tok::tk_subeq: new_assignment->set_type(assignment::kind::subeq); break;
            case tok::tk_multeq: new_assignment->set_type(assignment::kind::multeq); break;
            case tok::tk_diveq: new_assignment->set_type(assignment::kind::diveq); break;
            case tok::tk_remeq: new_assignment->set_type(assignment::kind::remeq); break;
            case tok::tk_btandeq: new_assignment->set_type(assignment::kind::andeq); break;
            case tok::tk_btxoreq: new_assignment->set_type(assignment::kind::xoreq); break;
            case tok::tk_btoreq: new_assignment->set_type(assignment::kind::oreq); break;
            default: break;
        }
        match(toks[ptr].type);
        new_assignment->set_right(calculation_gen());
        update_location(new_assignment);
        return new_assignment;
    }
    update_location(result);
    return result;
}

type_def* parse::type_gen() {
    auto result = new type_def(toks[ptr].loc);
    result->set_name(identifier_gen());
    while(look_ahead(tok::tk_mult)) {
        result->add_pointer_level();
        match(tok::tk_mult);
    }
    update_location(result);
    return result;
}

enum_decl* parse::enum_gen() {
    auto result = new enum_decl(toks[ptr].loc);
    match(tok::tk_enum);
    result->set_name(identifier_gen());
    match(tok::tk_lbrace);
    while(look_ahead(tok::tk_id)) {
        auto name_node = identifier_gen();
        if (look_ahead(tok::tk_eq)) {
            match(tok::tk_eq);
            auto value_node = number_gen();
            result->add_member(name_node, value_node);
        } else {
            result->add_member(name_node, nullptr);
        }
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else if (look_ahead(tok::tk_id)) {
            err.err("parse", toks[ptr-1].loc, "expected ',' here.");
        }
    }
    match(tok::tk_rbrace);
    update_location(result);
    return result;
}

struct_field* parse::struct_field_gen() {
    auto result = new struct_field(toks[ptr].loc);
    result->set_name(identifier_gen());
    match(tok::tk_colon);
    result->set_type(type_gen());
    update_location(result);
    return result;
}

struct_decl* parse::struct_gen() {
    auto result = new struct_decl(toks[ptr].loc);
    match(tok::tk_stct);
    result->set_name(toks[ptr].str);
    match(tok::tk_id);
    match(tok::tk_lbrace);
    while(!look_ahead(tok::tk_rbrace)) {
        result->add_field(struct_field_gen());
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else if (look_ahead(tok::tk_id)) {
            err.err("parse", toks[ptr-1].loc, "expected ',' here.");
        } else {
            break;
        }
    }
    match(tok::tk_rbrace);
    update_location(result);
    return result;
}

param* parse::param_gen() {
    auto result = new param(toks[ptr].loc);
    result->set_name(identifier_gen());
    if (look_ahead(tok::tk_colon)) {
        match(tok::tk_colon);
        result->set_type(type_gen());
    }
    update_location(result);
    return result;
}

param_list* parse::param_list_gen() {
    auto result = new param_list(toks[ptr].loc);
    match(tok::tk_lcurve);
    while(!look_ahead(tok::tk_rcurve)) {
        result->add_param(param_gen());
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else {
            break;
        }
    }
    match(tok::tk_rcurve);
    update_location(result);
    return result;
}

func_decl* parse::function_gen() {
    auto result = new func_decl(toks[ptr].loc);
    match(tok::tk_func);
    result->set_name(toks[ptr].str);
    match(tok::tk_id);
    result->set_params(param_list_gen());
    if (!look_ahead(tok::tk_arrow)) {
        auto ret = new type_def(toks[ptr].loc);
        ret->set_name(new identifier(toks[ptr].loc, "void"));
        update_location(ret);
        result->set_return_type(ret);
    } else {
        match(tok::tk_arrow);
        result->set_return_type(type_gen());
    }
    if (look_ahead(tok::tk_semi)) {
        match(tok::tk_semi);
        update_location(result);
        return result;
    }
    result->set_code_block(block_gen(false));
    update_location(result);
    return result;
}

impl_struct* parse::impl_gen() {
    match(tok::tk_impl);
    auto result = new impl_struct(toks[ptr].loc, toks[ptr].str);
    match(tok::tk_id);
    match(tok::tk_lbrace);
    while(look_ahead(tok::tk_func)) {
        result->add_method(function_gen());
    }
    match(tok::tk_rbrace);
    update_location(result);
    return result;
}

use_stmt* parse::use_stmt_gen() {
    auto result = new use_stmt(toks[ptr].loc);
    match(tok::tk_use);
    result->add_path(identifier_gen());
    while(look_ahead(tok::tk_double_colon)) {
        match(tok::tk_double_colon);
        if (look_ahead(tok::tk_lbrace) || look_ahead(tok::tk_mult)) {
            break;
        }
        result->add_path(identifier_gen());
    }
    if (look_ahead(tok::tk_lbrace)) {
        match(tok::tk_lbrace);
        while(look_ahead(tok::tk_id)) {
            result->add_import_symbol(identifier_gen());
            if (look_ahead(tok::tk_comma)) {
                match(tok::tk_comma);
            } else {
                break;
            }
        }
        match(tok::tk_rbrace);
    } else if (!look_ahead(tok::tk_mult)) {
        auto tmp = result->get_module_path().back();
        result->get_module_path().pop_back();
        result->add_import_symbol(tmp);
    } else {
        match(tok::tk_mult);
    }
    update_location(result);
    match(tok::tk_semi);
    return result;
}

definition* parse::definition_gen() {
    const auto& begin_location = toks[ptr].loc;
    match(tok::tk_var);
    auto result = new definition(begin_location, toks[ptr].str);
    match(tok::tk_id);
    if (look_ahead(tok::tk_colon)) {
        match(tok::tk_colon);
        result->set_type(type_gen());
    }
    match(tok::tk_eq);
    result->set_init_value(calculation_gen());
    match(tok::tk_semi);
    update_location(result);
    return result;
}

cond_stmt* parse::cond_stmt_gen() {
    auto result = new cond_stmt(toks[ptr].loc);
    auto new_if = new if_stmt(toks[ptr].loc);
    match(tok::tk_if);
    match(tok::tk_lcurve);
    new_if->set_condition(calculation_gen());
    match(tok::tk_rcurve);
    new_if->set_block(block_gen(true));
    update_location(new_if);
    result->add_stmt(new_if);
    while(look_ahead(tok::tk_elsif)) {
        auto new_elsif = new if_stmt(toks[ptr].loc);
        match(tok::tk_elsif);
        match(tok::tk_lcurve);
        new_elsif->set_condition(calculation_gen());
        match(tok::tk_rcurve);
        new_elsif->set_block(block_gen(true));
        update_location(new_elsif);
        result->add_stmt(new_elsif);
    }
    if (look_ahead(tok::tk_else)) {
        auto new_else = new if_stmt(toks[ptr].loc);
        match(tok::tk_else);
        new_else->set_block(block_gen(true));
        update_location(new_else);
        result->add_stmt(new_else);
    }
    update_location(result);
    return result;
}

match_stmt* parse::match_stmt_gen() {
    auto result = new match_stmt(toks[ptr].loc);
    match(tok::tk_match);
    match(tok::tk_lcurve);
    result->set_value(calculation_gen());
    match(tok::tk_rcurve);

    match(tok::tk_lbrace);
    while(!look_ahead(tok::tk_rbrace)) {
        auto new_case = new match_case(toks[ptr].loc);
        new_case->set_value(call_gen());
        match(tok::tk_wide_arrow);
        new_case->set_block(block_gen(true));
        update_location(new_case);
        result->add_case(new_case);
    }
    match(tok::tk_rbrace);
    update_location(result);
    return result;
}

while_stmt* parse::while_stmt_gen() {
    auto result = new while_stmt(toks[ptr].loc);
    match(tok::tk_while);
    match(tok::tk_lcurve);
    result->set_condition(calculation_gen());
    match(tok::tk_rcurve);
    result->set_block(block_gen(true));
    update_location(result);
    return result;
}

for_stmt* parse::for_stmt_gen() {
    auto result = new for_stmt(toks[ptr].loc);
    match(tok::tk_for);
    match(tok::tk_lcurve);
    if (look_ahead(tok::tk_var)) {
        result->set_init(definition_gen());
    } else {
        match(tok::tk_semi);
    }
    if (!look_ahead(tok::tk_semi)) {
        result->set_condition(calculation_gen());
    }
    match(tok::tk_semi);
    if (!look_ahead(tok::tk_rcurve)) {
        result->set_update(calculation_gen());
    }
    match(tok::tk_rcurve);
    result->set_block(block_gen(true));
    update_location(result);
    return result;
}

void parse::add_gen_stmt(code_block* result) {
    switch(toks[ptr].type) {
        case tok::tk_var: result->add_stmt(definition_gen()); break;
        case tok::tk_lcurve:
        case tok::tk_nil:
        case tok::tk_num:
        case tok::tk_str:
        case tok::tk_id: result->add_stmt(in_stmt_expr_gen()); break;
        case tok::tk_if: result->add_stmt(cond_stmt_gen()); break;
        case tok::tk_match: result->add_stmt(match_stmt_gen()); break;
        case tok::tk_while: result->add_stmt(while_stmt_gen()); break;
        case tok::tk_for: result->add_stmt(for_stmt_gen()); break;
        case tok::tk_ret: result->add_stmt(return_gen()); break;
        case tok::tk_cont: result->add_stmt(continue_gen()); break;
        case tok::tk_brk: result->add_stmt(break_gen()); break;
        case tok::tk_semi: match(tok::tk_semi); break;
        default:
            err.err("parse",
                toks[ptr].loc,
                "unexpected token for statement syntax \"" + toks[ptr].str + "\"."
            );
            match(toks[ptr].type);
            break;
    }
}

code_block* parse::block_gen(bool flag_allow_single_stmt_without_brace) {
    auto result = new code_block(toks[ptr].loc);

    if (flag_allow_single_stmt_without_brace && !look_ahead(tok::tk_lbrace)) {
        add_gen_stmt(result);
        update_location(result);
        return result;
    }

    match(tok::tk_lbrace);
    while(!look_ahead(tok::tk_rbrace)) {
        add_gen_stmt(result);
        if (look_ahead(tok::tk_eof)) {
            break;
        }
    }
    match(tok::tk_rbrace);
    update_location(result);
    return result;
}

in_stmt_expr* parse::in_stmt_expr_gen() {
    auto result = new in_stmt_expr(toks[ptr].loc);
    result->set_expr(calculation_gen());
    match(tok::tk_semi);
    update_location(result);
    return result;
}

ret_stmt* parse::return_gen() {
    auto result = new ret_stmt(toks[ptr].loc);
    match(tok::tk_ret);
    if (!look_ahead(tok::tk_semi)) {
        result->set_value(calculation_gen());
    }
    match(tok::tk_semi);
    update_location(result);
    return result;
}

continue_stmt* parse::continue_gen() {
    auto result = new continue_stmt(toks[ptr].loc);
    match(tok::tk_cont);
    match(tok::tk_semi);
    return result;
}

break_stmt* parse::break_gen() {
    auto result = new break_stmt(toks[ptr].loc);
    match(tok::tk_brk);
    match(tok::tk_semi);
    return result;
}

const error& parse::analyse(const std::vector<token>& token_list) {
    ptr = 0;
    toks = token_list.data();
    result = new root(token_list[0].loc);
    if (look_ahead(tok::tk_eof)) {
        return err;
    }
    while (look_ahead(tok::tk_use)) {
        result->add_use_stmt(use_stmt_gen());
    }
    while (!look_ahead(tok::tk_eof)) {
        switch(toks[ptr].type) {
            case tok::tk_func: result->add_decl(function_gen()); break;
            case tok::tk_stct: result->add_decl(struct_gen()); break;
            case tok::tk_impl: result->add_decl(impl_gen()); break;
            case tok::tk_enum: result->add_decl(enum_gen()); break;
            default:
                err.err("parse", toks[ptr].loc,
                    "unexpected token \"" + toks[ptr].str + "\"."
                );
                match(toks[ptr].type);
                break;
        }    
    }
    update_location(result);
    return err;
}

}