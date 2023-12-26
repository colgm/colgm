#include "ast/decl.h"
#include "ast/visitor.h"

namespace colgm {

void decl::accept(visitor* v) {
    v->visit_decl(this);
}

type_def::~type_def() {
    delete name;
}

void type_def::accept(visitor* v) {
    v->visit_type_def(this);
}

param::~param() {
    delete name;
    delete type;
}

void param::accept(visitor* v) {
    v->visit_param(this);
}

param_list::~param_list() {
    for(auto i : params) {
        delete i;
    }
}

void param_list::accept(visitor* v) {
    v->visit_param_list(this);
}

func_decl::~func_decl() {
    delete parameters;
    delete return_type;
    delete block;
}

void func_decl::accept(visitor* v) {
    v->visit_func_decl(this);
}

}