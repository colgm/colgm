#include "ast/decl.h"
#include "ast/visitor.h"

namespace colgm::ast {

void decl::accept(visitor* v) {
    v->visit_decl(this);
}

void cond_compile::accept(visitor* v) {
    v->visit_cond_compile(this);
}

cond_compile* cond_compile::clone() const {
    auto ret = new cond_compile(location, condition_name);
    for (const auto& i : conds) {
        ret->add_condition(i.first, i.second);
    }
    return ret;
}

generic_type_list::~generic_type_list() {
    for(auto i : types) {
        delete i;
    }
}

void generic_type_list::accept(visitor* v) {
    v->visit_generic_type_list(this);
}

generic_type_list* generic_type_list::clone() const {
    auto ret = new generic_type_list(location);
    for(auto i : types) {
        ret->add_type(i->clone());
    }
    return ret;
}

enum_decl::~enum_decl() {
    delete name;
    for (const auto& i : member) {
        delete i.name;
        delete i.value;
    }
    for (auto i : conds) {
        delete i;
    }
}

void enum_decl::accept(visitor* v) {
    v->visit_enum_decl(this);
}

enum_decl* enum_decl::clone() const {
    auto ret = new enum_decl(location);
    ret->name = name->clone();
    for (const auto& i : member) {
        ret->add_member(i.name->clone(), i.value? i.value->clone():nullptr);
    }
    for (auto i : conds) {
        ret->add_cond(i->clone());
    }
    ret->is_public = is_public;
    return ret;
}

type_def::~type_def() {
    delete name;
    delete generic_types;

    delete array_length;
}

void type_def::accept(visitor* v) {
    v->visit_type_def(this);
}

type_def* type_def::clone() const {
    auto ret = new type_def(location);
    ret->name = name->clone();
    if (generic_types) {
        ret->generic_types = generic_types->clone();
    }
    ret->pointer_depth = pointer_depth;
    ret->is_const = is_const;

    if (array_length) {
        ret->array_length = array_length->clone();
    }
    ret->is_array = is_array;
    return ret;
}

struct_field::~struct_field() {
    delete name;
    delete type;
}

void struct_field::accept(visitor* v) {
    v->visit_struct_field(this);
}

struct_field* struct_field::clone() const {
    auto ret = new struct_field(location);
    ret->name = name->clone();
    ret->type = type->clone();
    return ret;
}

struct_decl::~struct_decl() {
    delete generic_types;
    for (auto i : fields) {
        delete i;
    }
    for (auto i : conds) {
        delete i;
    }
}

void struct_decl::accept(visitor* v) {
    v->visit_struct_decl(this);
}

struct_decl* struct_decl::clone() const {
    auto ret = new struct_decl(location);
    ret->name = name;
    if (generic_types) {
        ret->generic_types = generic_types->clone();
    }
    for (auto i : fields) {
        ret->add_field(i->clone());
    }
    for (auto i : conds) {
        ret->add_cond(i->clone());
    }
    ret->is_public = is_public;
    ret->is_extern = is_extern;
    return ret;
}

param::~param() {
    delete name;
    delete type;
}

void param::accept(visitor* v) {
    v->visit_param(this);
}

param* param::clone() const {
    auto ret = new param(location);
    ret->name = name->clone();
    if (type) {
        ret->type = type->clone();
    }
    return ret;
}

param_list::~param_list() {
    for(auto i : params) {
        delete i;
    }
}

void param_list::accept(visitor* v) {
    v->visit_param_list(this);
}

param_list* param_list::clone() const {
    auto ret = new param_list(location);
    for(auto i : params) {
        ret->add_param(i->clone());
    }
    return ret;
}

func_decl::~func_decl() {
    delete generic_types;
    delete parameters;
    delete return_type;
    delete block;
    for (auto i : conds) {
        delete i;
    }
}

void func_decl::accept(visitor* v) {
    v->visit_func_decl(this);
}

func_decl* func_decl::clone() const {
    auto ret = new func_decl(location);
    ret->name = name;
    if (generic_types) {
        ret->generic_types = generic_types->clone();
    }
    ret->parameters = parameters->clone();
    if (return_type) {
        ret->return_type = return_type->clone();
    }
    if (block) {
        ret->block = block->clone();
    }
    for (auto i : conds) {
        ret->add_cond(i->clone());
    }
    ret->is_public = is_public;
    ret->is_extern = is_extern;
    return ret;
}

std::string func_decl::get_monomorphic_name() const {
    std::string trivial_cond = "";
    std::string non_trivial_cond = "";
    for (auto i : conds) {
        if (i->condition_name == "is_trivial") {
            trivial_cond = i->get_text();
        } else if (i->condition_name == "is_non_trivial") {
            non_trivial_cond = i->get_text();
        }
    }
    auto res = name;
    if (!trivial_cond.empty()) {
        res = trivial_cond + " " + res;
    }
    if (!non_trivial_cond.empty()) {
        res = non_trivial_cond + " " + res;
    }
    return res;
}

bool func_decl::contain_trivial_cond() const {
    for (auto i : conds) {
        if (i->condition_name == "is_trivial" ||
            i->condition_name == "is_non_trivial") {
            return true;
        }
    }
    return false;
}

impl_struct::~impl_struct() {
    delete generic_types;
    for (auto i : methods) {
        delete i;
    }
    for (auto i : conds) {
        delete i;
    }
}

void impl_struct::accept(visitor* v) {
    v->visit_impl_struct(this);
}

impl_struct* impl_struct::clone() const {
    auto ret = new impl_struct(location, struct_name);
    if (generic_types) {
        ret->generic_types = generic_types->clone();
    }
    for (auto i : methods) {
        ret->add_method(i->clone());
    }
    for (auto i : conds) {
        ret->add_cond(i->clone());
    }
    return ret;
}

}