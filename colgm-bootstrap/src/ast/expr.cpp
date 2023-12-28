#include "ast/expr.h"
#include "ast/visitor.h"

namespace colgm {

void expr::accept(visitor* v) {
    v->visit_expr(this);
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

call_field::~call_field() {
    delete func_call_args;
}

void call_field::accept(visitor* v) {
    v->visit_call_field(this);
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

}