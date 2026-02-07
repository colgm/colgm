#include "parse/parse.h"
#include "ast/delete_disabled_node.h"
#include "ast/replace_defer.h"

namespace colgm {

bool generic_list_look_ahead::look_ahead_type_def() {
    if (look_ahead(tok::tk_const)) {
        next();
    }
    if (look_ahead(tok::tk_id)) {
        next();
    } else {
        return false;
    }
    if (look_ahead(tok::tk_less) && !check()) {
        return false;
    }
    while (look_ahead(tok::tk_mult)) {
        next();
    }
    return true;
}

bool generic_list_look_ahead::check() {
    if (!look_ahead(tok::tk_less)) {
        return false;
    } else {
        next();
    }
    while (!look_ahead(tok::tk_grt)) {
        if (look_ahead(tok::tk_eof)) {
            return false;
        }
        if (!look_ahead_type_def()) {
            return false;
        }
        if (!look_ahead(tok::tk_comma) && !look_ahead(tok::tk_grt)) {
            return false;
        }
        if (look_ahead(tok::tk_comma)) {
            next();
        }
    }
    if (look_ahead(tok::tk_grt)) {
        next();
    }
    return true;
}

void parse::try_match_semi() {
    if (look_ahead(tok::tk_semi)) {
        next();
        return;
    }
    err.err(toks[ptr ? ptr - 1 : ptr].loc, "expected \";\"");
}

void parse::try_match_rbrace() {
    if (look_ahead(tok::tk_rbrace)) {
        next();
        return;
    }
    err.err(toks[ptr ? ptr - 1 : ptr].loc, "expected \"}\"");
}

void parse::match(tok type) {
    if (type == toks[ptr].type) {
        next();
        return;
    }
    switch(type) {
        case tok::tk_id:
            err.err(
                toks[ptr].loc,
                "expected identifier, but found \"" + toks[ptr].str + "\""
            );
            break;
        case tok::tk_num:
            err.err(
                toks[ptr].loc,
                "expected number, but found \"" + toks[ptr].str + "\""
            );
            break;
        case tok::tk_str:
            err.err(
                toks[ptr].loc,
                "expected string, but found \"" + toks[ptr].str + "\""
            );
            break;
        default:
            err.err(
                toks[ptr].loc,
                "expected \"" + tokname.at(type) +
                "\", but found \"" + toks[ptr].str + "\""
            );
            break;
    }
    // should be panic mode here, but this is a bootstrap
    // so do not do too many things
    next();
    return;
}

cond_compile* parse::conditional_compile() {
    auto begin_loc = toks[ptr].loc;
    match(tok::tk_sharp);
    match(tok::tk_lbracket);
    auto res = new cond_compile(begin_loc, toks[ptr].str);
    match(tok::tk_id);
    match(tok::tk_lcurve);
    while (!look_ahead(tok::tk_rcurve)) {
        auto key = toks[ptr].str;
        match(tok::tk_id);
        if (!look_ahead(tok::tk_eq)) {
            res->add_condition(key, "");
        } else {
            match(tok::tk_eq);
            auto value = toks[ptr].str;
            match(tok::tk_str);
            res->add_condition(key, value);
        }
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else {
            break;
        }
    }
    match(tok::tk_rcurve);
    match(tok::tk_rbracket);
    update_location(res);
    return res;
}

identifier* parse::identifier_gen() {
    auto res = new identifier(toks[ptr].loc, toks[ptr].str);
    match(tok::tk_id);
    return res;
}

call* parse::call_gen() {
    auto res = new call(toks[ptr].loc);
    res->set_head(call_id_gen());
    while (look_ahead(tok::tk_lcurve) || look_ahead(tok::tk_lbracket) ||
          look_ahead(tok::tk_dot) || look_ahead(tok::tk_arrow) ||
          look_ahead(tok::tk_double_colon) || look_ahead(tok::tk_lbrace)) {
        if (look_ahead(tok::tk_lcurve)) {
            match(tok::tk_lcurve);
            auto new_call_func = new call_func_args(toks[ptr].loc);
            while (!look_ahead(tok::tk_rcurve)) {
                new_call_func->add_arg(calculation_gen());
                if (look_ahead(tok::tk_comma)) {
                    match(tok::tk_comma);
                } else {
                    break;
                }
            }
            match(tok::tk_rcurve);
            update_location(new_call_func);
            res->add_chain(new_call_func);
        } else if (look_ahead(tok::tk_lbracket)) {
            match(tok::tk_lbracket);
            auto new_call_index = new call_index(toks[ptr].loc);
            new_call_index->set_index(calculation_gen());
            match(tok::tk_rbracket);
            update_location(new_call_index);
            res->add_chain(new_call_index);
        } else if (look_ahead(tok::tk_dot)) {
            const auto& dot_loc = toks[ptr].loc;
            match(tok::tk_dot);
            auto new_call_field = new get_field(toks[ptr].loc, dot_loc, toks[ptr].str);
            match(tok::tk_id);
            update_location(new_call_field);
            res->add_chain(new_call_field);
        } else if (look_ahead(tok::tk_arrow)) {
            const auto& arrow_loc = toks[ptr].loc;
            match(tok::tk_arrow);
            auto new_call_field = new ptr_get_field(toks[ptr].loc, arrow_loc, toks[ptr].str);
            match(tok::tk_id);
            update_location(new_call_field);
            res->add_chain(new_call_field);
        } else if (look_ahead(tok::tk_double_colon)) {
            match(tok::tk_double_colon);
            auto new_call_path = new call_path(toks[ptr].loc, toks[ptr].str);
            match(tok::tk_id);
            update_location(new_call_path);
            res->add_chain(new_call_path);
        } else if (look_ahead(tok::tk_lbrace)) {
            res->add_chain(initializer_gen());
        }
    }
    update_location(res);
    return res;
}

call_id* parse::call_id_gen() {
    auto res = new call_id(toks[ptr].loc);
    res->set_id(identifier_gen());
    if (look_ahead_generic()) {
        res->set_generic_types(generic_type_list_gen());
    }
    update_location(res);
    return res;
}

initializer* parse::initializer_gen() {
    auto res = new initializer(toks[ptr].loc);

    match(tok::tk_lbrace);
    while (!look_ahead(tok::tk_rbrace)) {
        auto pair_node = new init_pair(toks[ptr].loc);

        pair_node->set_field(identifier_gen());
        match(tok::tk_colon);
        pair_node->set_value(calculation_gen());
        update_location(pair_node);

        res->add_pair(pair_node);
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else {
            break;
        }
    }
    try_match_rbrace();

    update_location(res);
    return res;
}

nil_literal* parse::nil_gen() {
    auto res = new nil_literal(toks[ptr].loc);
    match(tok::tk_nil);
    return res;
}

number_literal* parse::number_gen() {
    auto res = new number_literal(toks[ptr].loc, toks[ptr].str);
    match(tok::tk_num);
    return res;
}

string_literal* parse::string_gen() {
    auto res = new string_literal(toks[ptr].loc, toks[ptr].str);
    match(tok::tk_str);
    return res;
}

char_literal* parse::char_gen() {
    auto res = new char_literal(toks[ptr].loc, toks[ptr].str[0]);
    match(tok::tk_ch);
    return res;
}

bool_literal* parse::bool_gen() {
    auto res = new bool_literal(toks[ptr].loc, toks[ptr].type == tok::tk_true);
    match(toks[ptr].type);
    return res;
}

array_list* parse::array_list_gen() {
    auto res = new array_list(toks[ptr].loc);
    match(tok::tk_lbracket);
    while (!look_ahead(tok::tk_rbracket)) {
        res->add_value(calculation_gen());
        if (look_ahead(tok::tk_eof)) {
            break;
        }
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else if (!look_ahead(tok::tk_rbracket)) {
            err.err(toks[ptr-1].loc, "expected ',' here");
        }
    }
    match(tok::tk_rbracket);
    update_location(res);
    return res;
}

expr* parse::scalar_gen() {
    if (look_ahead(tok::tk_lcurve)) {
        match(tok::tk_lcurve);
        auto res = calculation_gen();
        match(tok::tk_rcurve);
        return res;
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
        return array_list_gen();
    } else if (look_ahead(tok::tk_id)) {
        return call_gen();
    }
    err.err(toks[ptr].loc, "expected scalar here");
    next();
    return new null();
}

unary_operator* parse::unary_neg_gen() {
    auto res = new unary_operator(toks[ptr].loc);
    match(tok::tk_sub);
    res->set_value(scalar_gen());
    res->set_opr(unary_operator::kind::neg);
    update_location(res);
    return res;
}

unary_operator* parse::unary_bnot_gen() {
    auto res = new unary_operator(toks[ptr].loc);
    match(tok::tk_floater);
    res->set_value(scalar_gen());
    res->set_opr(unary_operator::kind::bnot);
    update_location(res);
    return res;
}

expr* parse::type_convert_gen() {
    const auto begin_location = toks[ptr].loc;
    auto res = scalar_gen();
    if (look_ahead(tok::tk_wide_arrow)) {
        match(tok::tk_wide_arrow);
        auto type_cast_node = new type_convert(begin_location);
        type_cast_node->set_source(res);
        type_cast_node->set_target(type_def_gen());
        res = type_cast_node;
    }

    update_location(res);
    return res;
}

expr* parse::multive_gen() {
    const auto begin_location = toks[ptr].loc;
    auto res = type_convert_gen();
    while (look_ahead(tok::tk_mult) ||
        look_ahead(tok::tk_div) ||
        look_ahead(tok::tk_rem)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(res);
        switch(toks[ptr].type) {
            case tok::tk_mult: binary->set_opr(binary_operator::kind::mult); break;
            case tok::tk_div: binary->set_opr(binary_operator::kind::div); break;
            case tok::tk_rem: binary->set_opr(binary_operator::kind::rem); break;
            default: break;
        }
        match(toks[ptr].type);
        binary->set_right(type_convert_gen());
        res = binary;
    }
    update_location(res);
    return res;
}

expr* parse::additive_gen() {
    const auto begin_location = toks[ptr].loc;
    auto res = multive_gen();
    while (look_ahead(tok::tk_add) || look_ahead(tok::tk_sub)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(res);
        switch(toks[ptr].type) {
            case tok::tk_add: binary->set_opr(binary_operator::kind::add); break;
            case tok::tk_sub: binary->set_opr(binary_operator::kind::sub); break;
            default: break;
        }
        match(toks[ptr].type);
        binary->set_right(multive_gen());
        res = binary;
    }
    update_location(res);
    return res;
}

expr* parse::bitwise_and_gen() {
    const auto begin_location = toks[ptr].loc;
    auto res = additive_gen();
    while (look_ahead(tok::tk_btand)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(res);
        binary->set_opr(binary_operator::kind::band);
        match(toks[ptr].type);
        binary->set_right(additive_gen());
        res = binary;
    }
    update_location(res);
    return res;
}

expr* parse::bitwise_xor_gen() {
    const auto begin_location = toks[ptr].loc;
    auto res = bitwise_and_gen();
    while (look_ahead(tok::tk_btxor)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(res);
        binary->set_opr(binary_operator::kind::bxor);
        match(toks[ptr].type);
        binary->set_right(bitwise_and_gen());
        res = binary;
    }
    update_location(res);
    return res;
}

expr* parse::bitwise_or_gen() {
    const auto begin_location = toks[ptr].loc;
    auto res = bitwise_xor_gen();
    while (look_ahead(tok::tk_btor)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(res);
        binary->set_opr(binary_operator::kind::bor);
        match(toks[ptr].type);
        binary->set_right(bitwise_xor_gen());
        res = binary;
    }
    update_location(res);
    return res;
}

expr* parse::compare_gen() {
    const auto begin_location = toks[ptr].loc;
    auto res = bitwise_or_gen();
    while (look_ahead(tok::tk_cmpeq) || look_ahead(tok::tk_neq) ||
        look_ahead(tok::tk_less) || look_ahead(tok::tk_leq) ||
        look_ahead(tok::tk_grt) || look_ahead(tok::tk_geq)) {
        auto binary = new binary_operator(begin_location);
        binary->set_left(res);
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
        res = binary;
    }
    update_location(res);
    return res;
}

expr* parse::not_expression_gen() {
    const auto begin_location = toks[ptr].loc;
    if (!look_ahead(tok::tk_opnot)) {
        return compare_gen();
    }
    match(tok::tk_opnot);
    auto value = compare_gen();
    auto res = new unary_operator(begin_location);
    res->set_opr(unary_operator::kind::lnot);
    res->set_value(value);
    update_location(res);
    return res;
}

expr* parse::and_expression_gen() {
    const auto begin_location = toks[ptr].loc;
    auto res = not_expression_gen();
    if (!look_ahead(tok::tk_opand)) {
        return res;
    }

    auto binary = new binary_operator(begin_location);
    binary->set_left(res);
    binary->set_opr(binary_operator::kind::cmpand);
    match(tok::tk_opand);
    binary->set_right(and_expression_gen());
    res = binary;
    update_location(res);
    return res;
}

expr* parse::or_expression_gen() {
    const auto begin_location = toks[ptr].loc;
    auto res = and_expression_gen();
    if (!look_ahead(tok::tk_opor)) {
        return res;
    }

    auto binary = new binary_operator(begin_location);
    binary->set_left(res);
    binary->set_opr(binary_operator::kind::cmpor);
    match(tok::tk_opor);
    binary->set_right(or_expression_gen());
    res = binary;
    update_location(res);
    return res;
}

expr* parse::calculation_gen() {
    const auto begin_location = toks[ptr].loc;
    auto res = or_expression_gen();
    if (!res) {
        return res;
    }
    if (!res->is(ast_type::ast_call)) {
        return res;
    }
    // if is call node, check assignment syntax
    if (look_ahead(tok::tk_addeq) || look_ahead(tok::tk_subeq) ||
        look_ahead(tok::tk_multeq) || look_ahead(tok::tk_diveq) ||
        look_ahead(tok::tk_remeq) || look_ahead(tok::tk_eq) ||
        look_ahead(tok::tk_btandeq) || look_ahead(tok::tk_btxoreq) ||
        look_ahead(tok::tk_btoreq)) {
        auto new_assignment = new assignment(begin_location);
        new_assignment->set_left(reinterpret_cast<call*>(res));
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
    update_location(res);
    return res;
}

type_def* parse::array_type_gen() {
    auto res = new type_def(toks[ptr].loc);
    match(tok::tk_lbracket);
    if (look_ahead(tok::tk_const)) {
        res->set_constant();
        match(tok::tk_const);
    }
    res->set_name(identifier_gen());
    if (look_ahead_generic()) {
        res->set_generic_types(generic_type_list_gen());
    }
    while (look_ahead(tok::tk_mult)) {
        res->add_pointer_level();
        match(tok::tk_mult);
    }
    match(tok::tk_semi);
    if (look_ahead(tok::tk_sub)) {
        err.err(toks[ptr].loc, "array type does not accept negative length");
        match(tok::tk_sub);
    }
    res->set_array(number_gen());
    match(tok::tk_rbracket);
    update_location(res);
    return res;
}

type_def* parse::type_def_gen() {
    if (look_ahead(tok::tk_lbracket)) {
        return array_type_gen();
    }

    auto res = new type_def(toks[ptr].loc);
    if (look_ahead(tok::tk_const)) {
        res->set_constant();
        match(tok::tk_const);
    }
    res->set_name(identifier_gen());
    // this is LL(k) look ahead
    // because in this case there's a confliction:
    //
    // foo => bar<T>
    //        ^^^^^^ type convert with <T>
    // foo => bar < exp
    //            ^^^^^^ compare expression `<`
    //
    if (look_ahead_generic()) {
        res->set_generic_types(generic_type_list_gen());
    }
    while (look_ahead(tok::tk_mult)) {
        res->add_pointer_level();
        match(tok::tk_mult);
    }
    if (look_ahead(tok::tk_btand)) {
        res->set_reference();
        match(tok::tk_btand);
    }
    update_location(res);
    return res;
}

generic_type_list* parse::generic_type_list_gen() {
    auto res = new generic_type_list(toks[ptr].loc);
    match(tok::tk_less);
    while (look_ahead(tok::tk_id)) {
        res->add_type(type_def_gen());
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else if (look_ahead(tok::tk_id)) {
            err.err(toks[ptr-1].loc, "expected ',' here");
        }
    }
    match(tok::tk_grt);
    update_location(res);
    return res;
}

enum_decl* parse::enum_gen(std::vector<cond_compile*>& conds,
                           bool flag_is_public,
                           bool flag_is_extern) {
    auto res = new enum_decl(toks[ptr].loc);
    for (auto i : conds) {
        res->add_cond(i);
    }
    if (flag_is_public) {
        res->set_public(true);
    }
    if (flag_is_extern) {
        err.err(toks[ptr].loc, "extern enum is not supported");
    }
    match(tok::tk_enum);
    res->set_name(identifier_gen());
    match(tok::tk_lbrace);
    while (look_ahead(tok::tk_id)) {
        auto name_node = identifier_gen();
        if (look_ahead(tok::tk_eq)) {
            match(tok::tk_eq);
            auto value_node = number_gen();
            res->add_member(name_node, value_node);
        } else {
            res->add_member(name_node, nullptr);
        }
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else if (look_ahead(tok::tk_id)) {
            err.err(toks[ptr-1].loc, "expected ',' here");
        }
    }
    try_match_rbrace();
    update_location(res);
    return res;
}

field_pair* parse::field_pair_gen() {
    auto res = new field_pair(toks[ptr].loc);
    res->set_name(identifier_gen());
    match(tok::tk_colon);
    res->set_type(type_def_gen());
    update_location(res);
    return res;
}

struct_decl* parse::struct_gen(std::vector<cond_compile*>& conds,
                               bool flag_is_public,
                               bool flag_is_extern) {
    auto res = new struct_decl(toks[ptr].loc);
    for (auto i : conds) {
        res->add_cond(i);
    }
    if (flag_is_public) {
        res->set_public(true);
    }
    if (flag_is_extern) {
        res->set_extern(true);
    }
    match(tok::tk_stct);
    res->set_name(toks[ptr].str);
    match(tok::tk_id);
    if (look_ahead_generic()) {
        res->set_generic_types(generic_type_list_gen());
    }
    match(tok::tk_lbrace);
    while (!look_ahead(tok::tk_rbrace)) {
        // error occurred
        if (!look_ahead(tok::tk_id)) {
            break;
        }
        res->add_field(field_pair_gen());
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else if (look_ahead(tok::tk_id)) {
            err.err(toks[ptr-1].loc, "expected ',' here");
        } else {
            break;
        }
    }
    try_match_rbrace();
    update_location(res);
    return res;
}

tagged_union_decl* parse::tagged_union_gen(std::vector<cond_compile*>& conds,
                                                bool flag_is_public,
                                                bool flag_is_extern) {
    auto res = new tagged_union_decl(toks[ptr].loc);
    for (auto i : conds) {
        res->add_cond(i);
    }
    if (flag_is_public) {
        res->set_public(true);
    }
    if (flag_is_extern) {
        res->set_extern(true);
    }

    match(tok::tk_union);
    match(tok::tk_lcurve);
    if (look_ahead(tok::tk_enum)) {
        match(tok::tk_enum);
    } else {
        res->set_ref_enum_name(toks[ptr].str);
        match(tok::tk_id);
    }
    match(tok::tk_rcurve);

    res->set_name(toks[ptr].str);
    match(tok::tk_id);

    match(tok::tk_lbrace);
    while (!look_ahead(tok::tk_rbrace)) {
        // error occurred
        if (!look_ahead(tok::tk_id)) {
            break;
        }
        res->add_member(field_pair_gen());
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else if (look_ahead(tok::tk_id)) {
            err.err(toks[ptr-1].loc, "expected ',' here");
        } else {
            break;
        }
    }
    try_match_rbrace();
    update_location(res);
    return res;
}

param* parse::param_gen() {
    auto res = new param(toks[ptr].loc);
    res->set_name(identifier_gen());
    if (look_ahead(tok::tk_colon)) {
        match(tok::tk_colon);
        res->set_type(type_def_gen());
    }
    update_location(res);
    return res;
}

param_list* parse::param_list_gen() {
    auto res = new param_list(toks[ptr].loc);
    match(tok::tk_lcurve);
    while (!look_ahead(tok::tk_rcurve)) {
        res->add_param(param_gen());
        if (look_ahead(tok::tk_comma)) {
            match(tok::tk_comma);
        } else {
            break;
        }
    }
    match(tok::tk_rcurve);
    update_location(res);
    return res;
}

func_decl* parse::function_gen(std::vector<cond_compile*>& conds,
                               bool flag_is_public,
                               bool flag_is_extern) {
    auto res = new func_decl(toks[ptr].loc);
    for (auto i : conds) {
        res->add_cond(i);
    }
    if (flag_is_public) {
        res->set_public(true);
    }
    if (flag_is_extern) {
        res->set_extern(true);
    }
    match(tok::tk_func);
    res->set_name(toks[ptr].str);
    match(tok::tk_id);
    if (look_ahead_generic()) {
        res->set_generic_types(generic_type_list_gen());
    }
    res->set_params(param_list_gen());
    if (!look_ahead(tok::tk_arrow)) {
        auto ret = new type_def(toks[ptr].loc);
        ret->set_name(new identifier(toks[ptr].loc, "void"));
        update_location(ret);
        res->set_return_type(ret);
    } else {
        match(tok::tk_arrow);
        res->set_return_type(type_def_gen());
    }
    if (look_ahead(tok::tk_semi)) {
        match(tok::tk_semi);
        update_location(res);
        return res;
    }
    res->set_code_block(block_gen(false));
    update_location(res);
    return res;
}

impl* parse::impl_gen(std::vector<cond_compile*>& conds,
                      bool flag_is_public,
                      bool flag_is_extern) {
    if (flag_is_public) {
        err.err(toks[ptr].loc, "\"pub\" is not used for impl struct");
    }
    if (flag_is_extern) {
        err.err(toks[ptr].loc, "\"extern\" is not used for impl struct");
    }
    match(tok::tk_impl);
    auto res = new impl(toks[ptr].loc, toks[ptr].str);
    for (auto i : conds) {
        res->add_cond(i);
    }
    match(tok::tk_id);
    if (look_ahead_generic()) {
        res->set_generic_types(generic_type_list_gen());
    }
    match(tok::tk_lbrace);
    while (look_ahead(tok::tk_sharp) ||
           look_ahead(tok::tk_func) ||
           look_ahead(tok::tk_pub)) {
        std::vector<cond_compile*> func_conds;
        while (look_ahead(tok::tk_sharp)) {
            func_conds.push_back(conditional_compile());
        }
        bool is_pub = false;
        if (look_ahead(tok::tk_pub)) {
            match(tok::tk_pub);
            is_pub = true;
        }
        auto func = function_gen(func_conds, is_pub, false);
        res->add_method(func);
    }
    try_match_rbrace();
    update_location(res);
    return res;
}

use_stmt* parse::use_stmt_gen() {
    auto res = new use_stmt(toks[ptr].loc);
    match(tok::tk_use);
    res->add_path(identifier_gen());
    while (look_ahead(tok::tk_double_colon)) {
        match(tok::tk_double_colon);
        if (look_ahead(tok::tk_lbrace) || look_ahead(tok::tk_mult)) {
            break;
        }
        res->add_path(identifier_gen());
    }
    if (look_ahead(tok::tk_lbrace)) {
        match(tok::tk_lbrace);
        while (look_ahead(tok::tk_id)) {
            res->add_import_symbol(identifier_gen());
            if (look_ahead(tok::tk_comma)) {
                match(tok::tk_comma);
            } else {
                break;
            }
        }
        try_match_rbrace();
    } else if (!look_ahead(tok::tk_mult)) {
        auto tmp = res->get_module_path().back();
        res->get_module_path().pop_back();
        res->add_import_symbol(tmp);
    } else {
        match(tok::tk_mult);
    }
    update_location(res);
    try_match_semi();
    return res;
}

definition* parse::definition_gen() {
    const auto& begin_location = toks[ptr].loc;
    match(tok::tk_var);
    auto res = new definition(begin_location, toks[ptr].str);
    match(tok::tk_id);
    if (look_ahead(tok::tk_colon)) {
        match(tok::tk_colon);
        res->set_type(type_def_gen());
    }
    match(tok::tk_eq);
    res->set_init_value(calculation_gen());
    try_match_semi();
    update_location(res);
    return res;
}

cond_stmt* parse::cond_stmt_gen() {
    auto res = new cond_stmt(toks[ptr].loc);
    auto new_if = new if_stmt(toks[ptr].loc);
    match(tok::tk_if);
    match(tok::tk_lcurve);
    new_if->set_condition(calculation_gen());
    match(tok::tk_rcurve);
    new_if->set_block(block_gen(true));
    update_location(new_if);
    res->add_stmt(new_if);
    while (look_ahead(tok::tk_elsif)) {
        auto new_elsif = new if_stmt(toks[ptr].loc);
        match(tok::tk_elsif);
        match(tok::tk_lcurve);
        new_elsif->set_condition(calculation_gen());
        match(tok::tk_rcurve);
        new_elsif->set_block(block_gen(true));
        update_location(new_elsif);
        res->add_stmt(new_elsif);
    }
    if (look_ahead(tok::tk_else)) {
        auto new_else = new if_stmt(toks[ptr].loc);
        match(tok::tk_else);
        new_else->set_block(block_gen(true));
        update_location(new_else);
        res->add_stmt(new_else);
    }
    update_location(res);
    return res;
}

match_stmt* parse::match_stmt_gen() {
    auto res = new match_stmt(toks[ptr].loc);
    match(tok::tk_match);
    match(tok::tk_lcurve);
    res->set_value(calculation_gen());
    match(tok::tk_rcurve);

    match(tok::tk_lbrace);
    while (!look_ahead(tok::tk_rbrace)) {
        auto new_case = new match_case(toks[ptr].loc);
        new_case->set_value(call_gen());
        match(tok::tk_wide_arrow);
        new_case->set_block(block_gen(true));
        update_location(new_case);
        res->add_case(new_case);
    }
    try_match_rbrace();
    update_location(res);
    return res;
}

while_stmt* parse::while_stmt_gen() {
    auto res = new while_stmt(toks[ptr].loc);
    match(tok::tk_while);
    match(tok::tk_lcurve);
    res->set_condition(calculation_gen());
    match(tok::tk_rcurve);
    res->set_block(block_gen(true));
    update_location(res);
    return res;
}

for_stmt* parse::for_stmt_gen() {
    auto res = new for_stmt(toks[ptr].loc);
    match(tok::tk_for);
    match(tok::tk_lcurve);
    if (look_ahead(tok::tk_var)) {
        res->set_init(definition_gen());
    } else {
        match(tok::tk_semi);
    }
    if (!look_ahead(tok::tk_semi)) {
        res->set_condition(calculation_gen());
    }
    match(tok::tk_semi);
    if (!look_ahead(tok::tk_rcurve)) {
        res->set_update(calculation_gen());
    }
    match(tok::tk_rcurve);
    res->set_block(block_gen(true));
    update_location(res);
    return res;
}

forindex* parse::forindex_gen() {
    auto res = new forindex(toks[ptr].loc);
    match(tok::tk_forindex);
    match(tok::tk_lcurve);
    match(tok::tk_var);
    res->set_variable(identifier_gen());
    match(tok::tk_semi);
    res->set_container(call_gen());
    match(tok::tk_rcurve);
    res->set_body(block_gen(true));
    update_location(res);
    return res;
}

foreach* parse::foreach_gen() {
    auto res = new foreach(toks[ptr].loc);
    match(tok::tk_foreach);
    match(tok::tk_lcurve);
    match(tok::tk_var);
    res->set_variable(identifier_gen());
    match(tok::tk_semi);
    res->set_container(call_gen());
    match(tok::tk_rcurve);
    res->set_body(block_gen(true));
    update_location(res);
    return res;
}

void parse::add_gen_stmt(code_block* res) {
    switch(toks[ptr].type) {
        case tok::tk_var: res->add_stmt(definition_gen()); break;
        case tok::tk_lcurve:
        case tok::tk_nil:
        case tok::tk_num:
        case tok::tk_str:
        case tok::tk_id: res->add_stmt(in_stmt_expr_gen()); break;
        case tok::tk_defer: res->add_stmt(defer_gen()); break;
        case tok::tk_if: res->add_stmt(cond_stmt_gen()); break;
        case tok::tk_match: res->add_stmt(match_stmt_gen()); break;
        case tok::tk_while: res->add_stmt(while_stmt_gen()); break;
        case tok::tk_for: res->add_stmt(for_stmt_gen()); break;
        case tok::tk_forindex: res->add_stmt(forindex_gen()); break;
        case tok::tk_foreach: res->add_stmt(foreach_gen()); break;
        case tok::tk_ret: res->add_stmt(return_gen()); break;
        case tok::tk_cont: res->add_stmt(continue_gen()); break;
        case tok::tk_brk: res->add_stmt(break_gen()); break;
        case tok::tk_semi: match(tok::tk_semi); break;
        default:
            err.err(toks[ptr].loc,
                "unexpected token for statement syntax \"" + toks[ptr].str + "\""
            );
            match(toks[ptr].type);
            break;
    }
}

code_block* parse::block_gen(bool flag_allow_single_stmt_without_brace) {
    auto res = new code_block(toks[ptr].loc);

    if (flag_allow_single_stmt_without_brace && !look_ahead(tok::tk_lbrace)) {
        add_gen_stmt(res);
        update_location(res);
        return res;
    }

    match(tok::tk_lbrace);
    while (!look_ahead(tok::tk_rbrace)) {
        add_gen_stmt(res);
        if (look_ahead(tok::tk_eof)) {
            break;
        }
    }
    try_match_rbrace();
    update_location(res);
    return res;
}

in_stmt_expr* parse::in_stmt_expr_gen() {
    auto res = new in_stmt_expr(toks[ptr].loc);
    res->set_expr(calculation_gen());
    try_match_semi();
    update_location(res);
    return res;
}

defer_stmt* parse::defer_gen() {
    auto res = new defer_stmt(toks[ptr].loc);
    match(tok::tk_defer);
    res->set_block(block_gen(true));
    update_location(res);
    return res;
}

ret_stmt* parse::return_gen() {
    auto res = new ret_stmt(toks[ptr].loc);
    match(tok::tk_ret);
    if (!look_ahead(tok::tk_semi)) {
        res->set_value(calculation_gen());
    }
    try_match_semi();
    update_location(res);
    return res;
}

continue_stmt* parse::continue_gen() {
    auto res = new continue_stmt(toks[ptr].loc);
    match(tok::tk_cont);
    try_match_semi();
    return res;
}

break_stmt* parse::break_gen() {
    auto res = new break_stmt(toks[ptr].loc);
    match(tok::tk_brk);
    try_match_semi();
    return res;
}

const error& parse::analyse(const std::vector<token>& token_list) {
    ptr = 0;
    toks = token_list.data();
    result = new root(token_list[0].loc);
    if (look_ahead(tok::tk_eof)) {
        return err;
    }

    // generate imports
    while (look_ahead(tok::tk_use)) {
        result->add_use_stmt(use_stmt_gen());
    }

    // generate decls
    while (!look_ahead(tok::tk_eof)) {
        bool flag_is_public = false;
        bool flag_is_extern = false;

        // generate conditional compile
        std::vector<cond_compile*> conds;
        while (look_ahead(tok::tk_sharp)) {
            conds.push_back(conditional_compile());
        }

        while (look_ahead(tok::tk_pub) || look_ahead(tok::tk_extern)) {
            if (look_ahead(tok::tk_pub) && !flag_is_public) {
                flag_is_public = true;
            } else if (look_ahead(tok::tk_pub) && flag_is_public) {
                err.err(toks[ptr].loc,
                    "duplicate modifier \"pub\""
                );
            }
            if (look_ahead(tok::tk_extern) && !flag_is_extern) {
                flag_is_extern = true;
            } else if (look_ahead(tok::tk_extern) && flag_is_extern) {
                err.err(toks[ptr].loc,
                    "duplicate modifier \"extern\""
                );
            }
            match(toks[ptr].type);
        }

        switch(toks[ptr].type) {
            case tok::tk_func:
                result->add_decl(function_gen(conds, flag_is_public, flag_is_extern));
                break;
            case tok::tk_stct:
                result->add_decl(struct_gen(conds, flag_is_public, flag_is_extern));
                break;
            case tok::tk_union:
                result->add_decl(tagged_union_gen(conds, flag_is_public, flag_is_extern));
                break;
            case tok::tk_impl:
                result->add_decl(impl_gen(conds, flag_is_public, flag_is_extern));
                break;
            case tok::tk_enum:
                result->add_decl(enum_gen(conds, flag_is_public, flag_is_extern));
                break;
            default:
                err.err(toks[ptr].loc,
                    "unexpected token \"" + toks[ptr].str + "\""
                );
                match(toks[ptr].type);
                break;
        }
    }
    update_location(result);

    // delete disabled nodes marked by the conditional comments syntax
    delete_disabled_node ddn;
    ddn.scan(err, result);

    replace_defer rd(err);
    rd.scan(result);
    return err;
}

}