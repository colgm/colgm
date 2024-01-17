#include "parse.h"

namespace colgm {

void parse::match(tok type) {
    if (type==toks[ptr].type) {
        next();
        return;
    }
    switch(type) {
        case tok::id:
            err.err("parse",
                toks[ptr].loc,
                "expected identifier but get \"" + toks[ptr].str + "\"."
            );
            break;
        case tok::num:
            err.err("parse",
                toks[ptr].loc,
                "expected number but get \"" + toks[ptr].str + "\"."
            );
            break;
        case tok::str:
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
    match(tok::id);
    return result;
}

call* parse::call_gen() {
    auto result = new call(toks[ptr].loc);
    result->set_head(identifier_gen());
    while(look_ahead(tok::lcurve) || look_ahead(tok::lbracket) ||
          look_ahead(tok::dot) || look_ahead(tok::arrow) ||
          look_ahead(tok::double_colon)) {
        if (look_ahead(tok::lcurve)) {
            match(tok::lcurve);
            auto new_call_func = new call_func_args(toks[ptr].loc);
            while(!look_ahead(tok::rcurve)) {
                new_call_func->add_arg(calculation_gen());
                if (look_ahead(tok::comma)) {
                    match(tok::comma);
                } else {
                    break;
                }
            }
            match(tok::rcurve);
            update_location(new_call_func);
            result->add_chain(new_call_func);
        } else if (look_ahead(tok::lbracket)) {
            match(tok::lbracket);
            auto new_call_index = new call_index(toks[ptr].loc);
            new_call_index->set_index(calculation_gen());
            match(tok::rbracket);
            update_location(new_call_index);
            result->add_chain(new_call_index);
        } else if (look_ahead(tok::dot)) {
            match(tok::dot);
            auto new_call_field = new call_field(toks[ptr].loc, toks[ptr].str);
            match(tok::id);
            update_location(new_call_field);
            result->add_chain(new_call_field);
        } else if (look_ahead(tok::arrow)) {
            match(tok::arrow);
            auto new_call_field = new ptr_call_field(toks[ptr].loc, toks[ptr].str);
            match(tok::id);
            update_location(new_call_field);
            result->add_chain(new_call_field);
        } else if (look_ahead(tok::double_colon)) {
            match(tok::double_colon);
            auto new_call_path = new call_path(toks[ptr].loc, toks[ptr].str);
            match(tok::id);
            update_location(new_call_path);
            result->add_chain(new_call_path);
        }
    }
    update_location(result);
    return result;
}

number_literal* parse::number_gen() {
    auto result = new number_literal(toks[ptr].loc, toks[ptr].str);
    match(tok::num);
    return result;
}

string_literal* parse::string_gen() {
    auto result = new string_literal(toks[ptr].loc, toks[ptr].str);
    match(tok::str);
    return result;
}

bool_literal* parse::bool_gen() {
    auto result = new bool_literal(toks[ptr].loc, toks[ptr].type==tok::tktrue);
    match(toks[ptr].type);
    return result;
}

expr* parse::scalar_gen() {
    if (look_ahead(tok::lcurve)) {
        match(tok::lcurve);
        auto result = calculation_gen();
        match(tok::rcurve);
        return result;
    }
    if (look_ahead(tok::num)) {
        return number_gen();
    }
    if (look_ahead(tok::str)) {
        return string_gen();
    }
    if (look_ahead(tok::tktrue) || look_ahead(tok::tkfalse)) {
        return bool_gen();
    }
    if (look_ahead(tok::id)) {
        return call_gen();
    }
    err.err("parse", toks[ptr].loc, "expected scalar here.");
    next();
    return new null();
}

expr* parse::multive_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = scalar_gen();
    while(look_ahead(tok::mult) || look_ahead(tok::div) || look_ahead(tok::mult)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(result);
        switch(toks[ptr].type) {
            case tok::mult: binary->set_opr(binary_operator::kind::mult); break;
            case tok::div: binary->set_opr(binary_operator::kind::div); break;
            case tok::mod: binary->set_opr(binary_operator::kind::mod); break;
            default: break;
        }
        match(toks[ptr].type);
        binary->set_right(scalar_gen());
        result = binary;
    }
    update_location(result);
    return result;
}

expr* parse::additive_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = multive_gen();
    while(look_ahead(tok::add) || look_ahead(tok::sub)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(result);
        switch(toks[ptr].type) {
            case tok::add: binary->set_opr(binary_operator::kind::add); break;
            case tok::sub: binary->set_opr(binary_operator::kind::sub); break;
            default: break;
        }
        match(toks[ptr].type);
        binary->set_right(multive_gen());
        result = binary;
    }
    update_location(result);
    return result;
}

expr* parse::compare_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = additive_gen();
    while(look_ahead(tok::cmpeq) || look_ahead(tok::neq) ||
        look_ahead(tok::less) || look_ahead(tok::leq) ||
        look_ahead(tok::grt) || look_ahead(tok::geq)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(result);
        switch(toks[ptr].type) {
            case tok::cmpeq: binary->set_opr(binary_operator::kind::cmpeq); break;
            case tok::neq: binary->set_opr(binary_operator::kind::cmpneq); break;
            case tok::less: binary->set_opr(binary_operator::kind::less); break;
            case tok::leq: binary->set_opr(binary_operator::kind::leq); break;
            case tok::grt: binary->set_opr(binary_operator::kind::grt); break;
            case tok::geq: binary->set_opr(binary_operator::kind::geq); break;
            default: break;
        }
        match(toks[ptr].type);
        binary->set_right(additive_gen());
        result = binary;
    }
    update_location(result);
    return result;
}

expr* parse::and_expression_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = compare_gen();
    if (!look_ahead(tok::opand)) {
        return result;
    }

    auto binary = new binary_operator(begin_location);
    binary->set_left(result);
    binary->set_opr(binary_operator::kind::cmpand);
    match(tok::opand);
    binary->set_right(and_expression_gen());
    result = binary;
    update_location(result);
    return result;
}

expr* parse::or_expression_gen() {
    const auto begin_location = toks[ptr].loc;
    auto result = and_expression_gen();
    if (!look_ahead(tok::opor)) {
        return result;
    }

    auto binary = new binary_operator(begin_location);
    binary->set_left(result);
    binary->set_opr(binary_operator::kind::cmpor);
    match(tok::opor);
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
    if (look_ahead(tok::addeq) || look_ahead(tok::subeq) || look_ahead(tok::multeq) ||
        look_ahead(tok::diveq) || look_ahead(tok::modeq) || look_ahead(tok::eq)) {
        auto new_assignment = new assignment(begin_location);
        new_assignment->set_left(reinterpret_cast<call*>(result));
        switch(toks[ptr].type) {
            case tok::eq: new_assignment->set_type(assignment::kind::eq); break;
            case tok::addeq: new_assignment->set_type(assignment::kind::addeq); break;
            case tok::subeq: new_assignment->set_type(assignment::kind::subeq); break;
            case tok::multeq: new_assignment->set_type(assignment::kind::multeq); break;
            case tok::diveq: new_assignment->set_type(assignment::kind::diveq); break;
            case tok::modeq: new_assignment->set_type(assignment::kind::modeq); break;
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
    while(look_ahead(tok::mult)) {
        result->add_pointer_level();
        match(tok::mult);
    }
    update_location(result);
    return result;
}

struct_field* parse::struct_field_gen() {
    auto result = new struct_field(toks[ptr].loc);
    result->set_name(identifier_gen());
    match(tok::colon);
    result->set_type(type_gen());
    update_location(result);
    return result;
}

struct_decl* parse::struct_gen() {
    auto result = new struct_decl(toks[ptr].loc);
    match(tok::stct);
    result->set_name(toks[ptr].str);
    match(tok::id);
    match(tok::lbrace);
    while(!look_ahead(tok::rbrace)) {
        result->add_field(struct_field_gen());
        if (look_ahead(tok::comma)) {
            match(tok::comma);
        } else {
            break;
        }
    }
    match(tok::rbrace);
    update_location(result);
    return result;
}

param* parse::param_gen() {
    auto result = new param(toks[ptr].loc);
    result->set_name(identifier_gen());
    match(tok::colon);
    result->set_type(type_gen());
    update_location(result);
    return result;
}

param_list* parse::param_list_gen() {
    auto result = new param_list(toks[ptr].loc);
    match(tok::lcurve);
    while(!look_ahead(tok::rcurve)) {
        result->add_param(param_gen());
        if (look_ahead(tok::comma)) {
            match(tok::comma);
        } else {
            break;
        }
    }
    match(tok::rcurve);
    update_location(result);
    return result;
}

func_decl* parse::function_gen() {
    auto result = new func_decl(toks[ptr].loc);
    match(tok::func);
    result->set_name(toks[ptr].str);
    match(tok::id);
    result->set_params(param_list_gen());
    if (!look_ahead(tok::arrow)) {
        auto ret = new type_def(toks[ptr].loc);
        ret->set_name(new identifier(toks[ptr].loc, "void"));
        update_location(ret);
        result->set_return_type(ret);
    } else {
        match(tok::arrow);
        result->set_return_type(type_gen());
    }
    if (look_ahead(tok::semi)) {
        match(tok::semi);
        update_location(result);
        return result;
    }
    result->set_code_block(block_gen());
    update_location(result);
    return result;
}

impl_struct* parse::impl_gen() {
    match(tok::impl);
    auto result = new impl_struct(toks[ptr].loc, toks[ptr].str);
    match(tok::id);
    match(tok::lbrace);
    while(look_ahead(tok::func)) {
        result->add_method(function_gen());
    }
    match(tok::rbrace);
    update_location(result);
    return result;
}

use_stmt* parse::use_stmt_gen() {
    auto result = new use_stmt(toks[ptr].loc);
    match(tok::use);
    result->add_path(identifier_gen());
    while(look_ahead(tok::double_colon)) {
        match(tok::double_colon);
        if (look_ahead(tok::lbrace) || look_ahead(tok::mult)) {
            break;
        }
        result->add_path(identifier_gen());
    }
    if (look_ahead(tok::lbrace)) {
        match(tok::lbrace);
        while(look_ahead(tok::id)) {
            result->add_import_symbol(identifier_gen());
            if (look_ahead(tok::comma)) {
                match(tok::comma);
            } else {
                break;
            }
        }
        match(tok::rbrace);
    } else if (!look_ahead(tok::mult)) {
        auto tmp = result->get_module_path().back();
        result->get_module_path().pop_back();
        result->add_import_symbol(tmp);
    } else {
        match(tok::mult);
    }
    update_location(result);
    match(tok::semi);
    return result;
}

definition* parse::definition_gen() {
    match(tok::var);
    auto result = new definition(toks[ptr].loc, toks[ptr].str);
    match(tok::id);
    match(tok::colon);
    result->set_type(type_gen());
    match(tok::eq);
    result->set_init_value(calculation_gen());
    match(tok::semi);
    update_location(result);
    return result;
}

cond_stmt* parse::cond_stmt_gen() {
    auto result = new cond_stmt(toks[ptr].loc);
    auto new_if = new if_stmt(toks[ptr].loc);
    match(tok::rif);
    match(tok::lcurve);
    new_if->set_condition(calculation_gen());
    match(tok::rcurve);
    new_if->set_block(block_gen());
    update_location(new_if);
    result->add_stmt(new_if);
    while(look_ahead(tok::elsif)) {
        auto new_elsif = new if_stmt(toks[ptr].loc);
        match(tok::elsif);
        match(tok::lcurve);
        new_elsif->set_condition(calculation_gen());
        match(tok::rcurve);
        new_elsif->set_block(block_gen());
        update_location(new_elsif);
        result->add_stmt(new_elsif);
    }
    if (look_ahead(tok::relse)) {
        auto new_else = new if_stmt(toks[ptr].loc);
        match(tok::relse);
        new_else->set_block(block_gen());
        update_location(new_else);
        result->add_stmt(new_else);
    }
    update_location(result);
    return result;
}

while_stmt* parse::while_stmt_gen() {
    auto result = new while_stmt(toks[ptr].loc);
    match(tok::rwhile);
    match(tok::lcurve);
    result->set_condition(calculation_gen());
    match(tok::rcurve);
    result->set_block(block_gen());
    update_location(result);
    return result;
}

code_block* parse::block_gen() {
    auto result = new code_block(toks[ptr].loc);
    match(tok::lbrace);
    while(!look_ahead(tok::rbrace)) {
        switch(toks[ptr].type) {
            case tok::var: result->add_stmt(definition_gen()); break;
            case tok::lcurve:
            case tok::num:
            case tok::str:
            case tok::id: result->add_stmt(in_stmt_expr_gen()); break;
            case tok::rif: result->add_stmt(cond_stmt_gen()); break;
            case tok::rwhile: result->add_stmt(while_stmt_gen()); break;
            case tok::ret: result->add_stmt(return_gen()); break;
            default: match(toks[ptr].type); break;
        }
    }
    match(tok::rbrace);
    update_location(result);
    return result;
}

in_stmt_expr* parse::in_stmt_expr_gen() {
    auto result = new in_stmt_expr(toks[ptr].loc);
    result->set_expr(calculation_gen());
    match(tok::semi);
    update_location(result);
    return result;
}

ret_stmt* parse::return_gen() {
    auto result = new ret_stmt(toks[ptr].loc);
    match(tok::ret);
    if (!look_ahead(tok::semi)) {
        result->set_value(calculation_gen());
    }
    match(tok::semi);
    update_location(result);
    return result;
}

const error& parse::analyse(const std::vector<token>& token_list) {
    ptr = 0;
    toks = token_list.data();
    result = new root(token_list[0].loc);
    if (look_ahead(tok::eof)) {
        return err;
    }
    while (look_ahead(tok::use)) {
        result->add_use_stmt(use_stmt_gen());
    }
    while (!look_ahead(tok::eof)) {
        switch(toks[ptr].type) {
            case tok::func: result->add_decl(function_gen()); break;
            case tok::stct: result->add_decl(struct_gen()); break;
            case tok::impl: result->add_decl(impl_gen()); break;
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