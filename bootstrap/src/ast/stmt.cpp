#include "ast/stmt.h"
#include "ast/visitor.h"

namespace colgm::ast {

void stmt::accept(visitor* v) {
    v->visit_stmt(this);
}

use_stmt::~use_stmt() {
    for(auto i : module_path) {
        delete i;
    }
    for(auto i : import_symbol) {
        delete i;
    }
}

void use_stmt::accept(visitor* v) {
    v->visit_use_stmt(this);
}

use_stmt* use_stmt::clone() const {
    auto ret = new use_stmt(location);
    for(auto i : module_path) {
        ret->add_path(i->clone());
    }
    for(auto i : import_symbol) {
        ret->add_import_symbol(i->clone());
    }
    return ret;
}

definition::~definition() {
    delete type;
    delete init_value;
}

void definition::accept(visitor* v) {
    v->visit_definition(this);
}

definition* definition::clone() const {
    auto ret = new definition(location, name);
    if (type) {
        ret->type = type->clone();
    }
    if (init_value) {
        ret->init_value = init_value->clone();
    }
    return ret;
}

cond_stmt::~cond_stmt() {
    for(auto i : stmts) {
        delete i;
    }
}

void cond_stmt::accept(visitor* v) {
    v->visit_cond_stmt(this);
}

cond_stmt* cond_stmt::clone() const {
    auto ret = new cond_stmt(location);
    for(auto i : stmts) {
        ret->add_stmt(i->clone());
    }
    return ret;
}

if_stmt::~if_stmt() {
    delete condition;
    delete block;
}

void if_stmt::accept(visitor* v) {
    v->visit_if_stmt(this);
}

if_stmt* if_stmt::clone() const {
    auto ret = new if_stmt(location);
    if (condition) {
        ret->condition = condition->clone();
    }
    if (block) {
        ret->block = block->clone();
    }
    return ret;
}

match_case::~match_case() {
    delete value;
    delete block;
}

void match_case::accept(visitor* v) {
    v->visit_match_case(this);
}

match_case* match_case::clone() const {
    auto ret = new match_case(location);
    if (value) {
        ret->value = value->clone();
    }
    if (block) {
        ret->block = block->clone();
    }
    return ret;
}

match_stmt::~match_stmt() {
    delete value;
    for(auto i : cases) {
        delete i;
    }
}

void match_stmt::accept(visitor* v) {
    v->visit_match_stmt(this);
}

match_stmt* match_stmt::clone() const {
    auto ret = new match_stmt(location);
    if (value) {
        ret->value = value->clone();
    }
    for(auto i : cases) {
        ret->add_case(i->clone());
    }
    return ret;
}

while_stmt::~while_stmt() {
    delete condition;
    delete block;
}

void while_stmt::accept(visitor* v) {
    v->visit_while_stmt(this);
}

while_stmt* while_stmt::clone() const {
    auto ret = new while_stmt(location);
    if (condition) {
        ret->condition = condition->clone();
    }
    if (block) {
        ret->block = block->clone();
    }
    return ret;
}

for_stmt::~for_stmt() {
    delete init;
    delete condition;
    delete update;
    delete block;
}

void for_stmt::accept(visitor* v) {
    v->visit_for_stmt(this);
}

for_stmt* for_stmt::clone() const {
    auto ret = new for_stmt(location);
    if (init) {
        ret->init = init->clone();
    }
    if (condition) {
        ret->condition = condition->clone();
    }
    if (update) {
        ret->update = update->clone();
    }
    if (block) {
        ret->block = block->clone();
    }
    return ret;
}

in_stmt_expr::~in_stmt_expr() {
    delete calculation;
}

void in_stmt_expr::accept(visitor* v) {
    v->visit_in_stmt_expr(this);
}

ret_stmt::~ret_stmt() {
    delete value;
}

void ret_stmt::accept(visitor* v) {
    v->visit_ret_stmt(this);
}

void continue_stmt::accept(visitor* v) {
    v->visit_continue_stmt(this);
}

void break_stmt::accept(visitor* v) {
    v->visit_break_stmt(this);
}

code_block::~code_block() {
    for(auto i : statements) {
        delete i;
    }
}

void code_block::accept(visitor* v) {
    v->visit_code_block(this);
}

code_block* code_block::clone() const {
    auto ret = new code_block(location);
    for(auto i : statements) {
        ret->add_stmt(i->clone());
    }
    return ret;
}

}