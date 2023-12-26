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

identifier* parse::identifier_gen() {
    auto result = new identifier(toks[ptr].loc, toks[ptr].str);
    match(tok::id);
    return result;
}

type_def* parse::type_gen() {
    auto result = new type_def(toks[ptr].loc);
    result->set_name(identifier_gen());
    while(look_ahead(tok::mult)) {
        result->add_pointer_level();
        match(tok::mult);
    }
    return result;
}

param* parse::param_gen() {
    auto result = new param(toks[ptr].loc);
    result->set_name(identifier_gen());
    match(tok::colon);
    result->set_type(type_gen());
    return result;
}

param_list* parse::param_list_gen() {
    auto result = new param_list(toks[ptr].loc);
    match(tok::lcurve);
    while(!look_ahead(tok::rcurve)) {
        result->add_param(param_gen());
        if (look_ahead(tok::comma)) {
            match(tok::comma);
        }
    }
    match(tok::rcurve);
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
        result->set_return_type(ret);
    } else {
        match(tok::arrow);
        result->set_return_type(type_gen());
    }
    result->set_code_block(new code_block(toks[ptr].loc));
    match(tok::lbrace);
    match(tok::rbrace);
    return result;
}

const error& parse::analyse(const std::vector<token>& token_list) {
    ptr = 0;
    toks = token_list.data();
    result = new root(token_list[0].loc);
    if (look_ahead(tok::eof)) {
        return err;
    }
    while (!look_ahead(tok::eof)) {
        if (look_ahead(tok::func)) {
            result->add_decl(function_gen());
            continue;
        }
        err.err("parse", toks[ptr].loc,
            "unknown token \"" + toks[ptr].str + "\"."
        );
        match(toks[ptr].type);
    }
    return err;
}

}