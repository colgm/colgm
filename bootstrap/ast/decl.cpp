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
    for (const auto& i : ordered_cond_name) {
        ret->add_condition(i, conds.at(i));
    }
    return ret;
}

generic_type_list::~generic_type_list() {
    for (auto i : types) {
        delete i;
    }
}

void generic_type_list::accept(visitor* v) {
    v->visit_generic_type_list(this);
}

generic_type_list* generic_type_list::clone() const {
    auto ret = new generic_type_list(location);
    for (auto i : types) {
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

field_pair::~field_pair() {
    delete name;
    delete type;
}

void field_pair::accept(visitor* v) {
    v->visit_field_pair(this);
}

field_pair* field_pair::clone() const {
    auto ret = new field_pair(location);
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

tagged_union_decl::~tagged_union_decl() {
    for (auto i : fields) {
        delete i;
    }
    for (auto i : conds) {
        delete i;
    }
}

void tagged_union_decl::accept(visitor* v) {
    v->visit_tagged_union_decl(this);
}

tagged_union_decl* tagged_union_decl::clone() const { 
    auto ret = new tagged_union_decl(location);
    ret->name = name;
    ret->ref_enum_name = ref_enum_name;
    for (auto i : fields) {
        ret->add_member(i->clone());
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
    for (auto i : params) {
        delete i;
    }
}

void param_list::accept(visitor* v) {
    v->visit_param_list(this);
}

param_list* param_list::clone() const {
    auto ret = new param_list(location);
    for (auto i : params) {
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
    std::string is_pointer_cond = "";
    std::string is_non_pointer_cond = "";
    for (auto i : conds) {
        if (i->get_condition_name() == "is_trivial") {
            trivial_cond = i->get_text();
        } else if (i->get_condition_name() == "is_non_trivial") {
            non_trivial_cond = i->get_text();
        } else if (i->get_condition_name() == "is_pointer") {
            is_pointer_cond = i->get_text();
        } else if (i->get_condition_name() == "is_non_pointer") {
            is_non_pointer_cond = i->get_text();
        }
    }
    auto res = name;
    if (!trivial_cond.empty()) {
        res = trivial_cond + " " + res;
    }
    if (!non_trivial_cond.empty()) {
        res = non_trivial_cond + " " + res;
    }
    if (!is_pointer_cond.empty()) {
        res = is_pointer_cond + " " + res;
    }
    if (!is_non_pointer_cond.empty()) {
        res = is_non_pointer_cond + " " + res;
    }
    return res;
}

bool func_decl::contain_cond() const {
    for (auto i : conds) {
        if (i->get_condition_name() == "is_trivial" ||
            i->get_condition_name() == "is_non_trivial" ||
            i->get_condition_name() == "is_pointer" ||
            i->get_condition_name() == "is_non_pointer") {
            return true;
        }
    }
    return false;
}

cond_compile* func_decl::get_trivial_cond() const { 
    for (auto i : conds) {
        if (i->get_condition_name() == "is_trivial") {
            return i;
        }
    }
    return nullptr;
}

cond_compile* func_decl::get_non_trivial_cond() const { 
    for (auto i : conds) {
        if (i->get_condition_name() == "is_non_trivial") {
            return i;
        }
    }
    return nullptr;
}

cond_compile* func_decl::get_is_pointer_cond() const { 
    for (auto i : conds) {
        if (i->get_condition_name() == "is_pointer") {
            return i;
        }
    }
    return nullptr;
}

cond_compile* func_decl::get_is_non_pointer_cond() const { 
    for (auto i : conds) {
        if (i->get_condition_name() == "is_non_pointer") {
            return i;
        }
    }
    return nullptr;
}

impl::~impl() {
    delete generic_types;
    for (auto i : methods) {
        delete i;
    }
    for (auto i : conds) {
        delete i;
    }
}

void impl::accept(visitor* v) {
    v->visit_impl(this);
}

impl* impl::clone() const {
    auto ret = new impl(location, name);
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