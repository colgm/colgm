#include "ast/expr.h"
#include "ast/visitor.h"

namespace colgm::ast {

void expr::accept(visitor* v) {
    v->visit_expr(this);
}

unary_operator::~unary_operator() {
    delete value;
}

void unary_operator::accept(visitor* v) {
    v->visit_unary_operator(this);
}

unary_operator* unary_operator::clone() const {
    auto copy = new unary_operator(location);
    copy->opr = opr;
    copy->value = value->clone();
    return copy;
}

binary_operator::~binary_operator() {
    delete left;
    delete right;
}

void binary_operator::accept(visitor* v) {
    v->visit_binary_operator(this);
}

binary_operator* binary_operator::clone() const {
    auto copy = new binary_operator(location);
    copy->opr = opr;
    copy->left = left->clone();
    copy->right = right->clone();
    return copy;
}

type_convert::~type_convert() {
    delete source;
    delete target;
}

void type_convert::accept(visitor* v) {
    v->visit_type_convert(this);
}

type_convert* type_convert::clone() const {
    auto copy = new type_convert(location);
    copy->source = source->clone();
    copy->target = target->clone();
    return copy;
}

void identifier::accept(visitor* v) {
    v->visit_identifier(this);
}

void nil_literal::accept(visitor* v) {
    v->visit_nil_literal(this);
}

void number_literal::accept(visitor* v) {
    v->visit_number_literal(this);
}

void string_literal::accept(visitor* v) {
    v->visit_string_literal(this);
}

void char_literal::accept(visitor* v) {
    v->visit_char_literal(this);
}

void bool_literal::accept(visitor* v) {
    v->visit_bool_literal(this);
}

array_list::~array_list() {
    for (auto i : value) {
        delete i;
    }
}

void array_list::accept(visitor* v) {
    v->visit_array_list(this);
}

array_list* array_list::clone() const {
    auto copy = new array_list(location);
    for (auto i : value) {
        copy->value.push_back(i->clone());
    }
    return copy;
}

call_id::~call_id() {
    delete id;
    delete generic_types;
}

void call_id::accept(visitor* v) {
    v->visit_call_id(this);
}

call_id* call_id::clone() const {
    auto copy = new call_id(location);
    copy->id = id->clone();
    if (generic_types) {
        copy->generic_types = generic_types->clone();
    }
    return copy;
}

call_index::~call_index() {
    delete index;
}

void call_index::accept(visitor* v) {
    v->visit_call_index(this);
}

call_index* call_index::clone() const {
    auto copy = new call_index(location);
    copy->index = index->clone();
    return copy;
}

call_func_args::~call_func_args() {
    for (auto i : args) {
        delete i;
    }
}

void call_func_args::accept(visitor* v) {
    v->visit_call_func_args(this);
}

call_func_args* call_func_args::clone() const {
    auto copy = new call_func_args(location);
    for (auto i : args) {
        copy->args.push_back(i->clone());
    }
    copy->args_is_ref = args_is_ref;
    return copy;
}

void get_field::accept(visitor* v) {
    v->visit_get_field(this);
}

void ptr_get_field::accept(visitor* v) {
    v->visit_ptr_get_field(this);
}

void init_pair::accept(visitor* v) {
    v->visit_init_pair(this);
}

init_pair* init_pair::clone() const {
    auto copy = new init_pair(location);
    copy->field = field->clone();
    copy->value = value->clone();
    return copy;
}

void initializer::accept(visitor* v) {
    v->visit_initializer(this);
}

initializer* initializer::clone() const {
    auto copy = new initializer(location);
    for (auto i : pairs) {
        copy->pairs.push_back(i->clone());
    }
    return copy;
}

void call_path::accept(visitor* v) {
    v->visit_call_path(this);
}

call::~call() {
    delete head;
    for (auto i : chain) {
        delete i;
    }
}

void call::accept(visitor* v) {
    v->visit_call(this);
}

call* call::clone() const {
    auto copy = new call(location);
    copy->head = head->clone();
    for (auto i : chain) {
        copy->chain.push_back(i->clone());
    }
    return copy;
}

assignment::~assignment() {
    delete left;
    delete right;
}

void assignment::accept(visitor* v) {
    v->visit_assignment(this);
}

assignment* assignment::clone() const {
    auto copy = new assignment(location);
    copy->set_type(type);
    copy->left = left->clone();
    copy->right = right->clone();
    return copy;
}

}