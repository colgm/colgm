#include "ast/expr.h"
#include "ast/visitor.h"

namespace colgm {

void expr::accept(visitor* v) {
    v->visit_expr(this);
}

binary_operator::~binary_operator() {
    delete left;
    delete right;
}

void binary_operator::accept(visitor* v) {
    v->visit_binary_operator(this);
}

void identifier::accept(visitor* v) {
    v->visit_identifier(this);
}

void number_literal::accept(visitor* v) {
    v->visit_number_literal(this);
}

void string_literal::accept(visitor* v) {
    v->visit_string_literal(this);
}

call_index::~call_index() {
    delete index;
}

void call_index::accept(visitor* v) {
    v->visit_call_index(this);
}

call_func_args::~call_func_args() {
    for(auto i : args) {
        delete i;
    }
}

void call_func_args::accept(visitor* v) {
    v->visit_call_func_args(this);
}

void call_field::accept(visitor* v) {
    v->visit_call_field(this);
}

void ptr_call_field::accept(visitor* v) {
    v->visit_ptr_call_field(this);
}

void call_path::accept(visitor* v) {
    v->visit_call_path(this);
}

call::~call() {
    delete head;
    for(auto i : chain) {
        delete i;
    }
}

void call::accept(visitor* v) {
    v->visit_call(this);
}

assignment::~assignment() {
    delete left;
    delete right;
}

void assignment::accept(visitor* v) {
    v->visit_assignment(this);
}

}