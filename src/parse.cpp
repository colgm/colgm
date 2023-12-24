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

bool parse::look_ahead_single_use() {
    return toks[ptr].type==tok::id && toks[ptr+1].type==tok::semi;
}

identifier* parse::identifier_gen() {
    auto result = new identifier(toks[ptr].loc, toks[ptr].str);
    match(tok::id);
    return result;
}

use_stmt* parse::use_statement_gen() {
    auto result = new use_stmt(toks[ptr].loc);
    match(tok::use);
    result->add_path(identifier_gen());
    while(look_ahead(tok::double_colon)) {
        match(tok::double_colon);
        if (look_ahead_single_use()) {
            break;
        }
        if (look_ahead(tok::id)) {
            result->add_path(identifier_gen());
        } else {
            break;
        }
    }
    if (look_ahead(tok::mult)) {
        match(tok::mult);
        result->set_use_kind(use_stmt::use_kind::use_all);
    } else if (look_ahead(tok::id)) {
        result->set_use_kind(use_stmt::use_kind::use_single);
        result->set_single_use(identifier_gen());
    } else if (look_ahead(tok::lbrace)) {
        match(tok::lbrace);
        result->set_use_kind(use_stmt::use_kind::use_specify);
        while(!look_ahead(tok::rbrace)) {
            result->add_specified_use(identifier_gen());
            if (!look_ahead(tok::rbrace) || look_ahead(tok::comma)) {
                match(tok::comma);
            }
            if (look_ahead(tok::eof)) {
                break;
            }
        }
        match(tok::rbrace);
    }
    match(tok::semi);
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
        if (look_ahead(tok::use)) {
            result->add_use_statement(use_statement_gen());
        } else {
            err.err("parse", toks[ptr].loc,
                "unknown token \"" + toks[ptr].str + "\"."
            );
            match(toks[ptr].type);
        }
    }
    return err;
}

}