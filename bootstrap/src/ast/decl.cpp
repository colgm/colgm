#include "ast/decl.h"
#include "ast/visitor.h"

namespace colgm::ast {

void decl::accept(visitor* v) {
    v->visit_decl(this);
}

enum_decl::~enum_decl() {
    delete name;
    for(auto i : member) {
        delete i;
    }
}

void enum_decl::accept(visitor* v) {
    v->visit_enum_decl(this);
}

type_def::~type_def() {
    delete name;
}

void type_def::accept(visitor* v) {
    v->visit_type_def(this);
}

struct_field::~struct_field() {
    delete name;
    delete type;
}

void struct_field::accept(visitor* v) {
    v->visit_struct_field(this);
}

struct_decl::~struct_decl() {
    for(auto i : fields) {
        delete i;
    }
}

void struct_decl::accept(visitor* v) {
    v->visit_struct_decl(this);
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

impl_struct::~impl_struct() {
    for(auto i : methods) {
        delete i;
    }
}

void impl_struct::accept(visitor* v) {
    v->visit_impl_struct(this);
}

}