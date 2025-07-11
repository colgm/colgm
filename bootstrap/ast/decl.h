#pragma once

#include "colgm.h"
#include "ast/ast.h"

#include <cstring>
#include <sstream>
#include <cassert>

namespace colgm::ast {

class decl: public node {
public:
    decl(ast_type type, const span& loc): node(type, loc) {}
    ~decl() override = default;
    void accept(visitor*) override;
    decl* clone() const override {
        assert(false && "not implemented: class decl");
        return nullptr;
    };
};

class cond_compile: public decl {
private:
    std::string condition_name;
    std::vector<std::string> ordered_cond_name;
    std::unordered_map<std::string, std::string> conds;

public:
    cond_compile(const span& loc, const std::string& n):
        decl(ast_type::ast_cond_compile, loc),
        condition_name(n) {}
    
    ~cond_compile() override = default;
    void accept(visitor*) override;
    cond_compile* clone() const override;

    void add_condition(const std::string& key, const std::string& value) {
       conds[key] = value;
       ordered_cond_name.push_back(key);
    }

    const auto& get_condition_name() const { return condition_name; }
    const auto& get_conds() const { return conds; }
    const auto& get_ordered_cond_name() const { return ordered_cond_name; }
    auto get_text() const {
        auto text = "#[" + condition_name + "(";
        for (const auto& i : ordered_cond_name) {
            text += i;
            if (!conds.at(i).empty()) {
                text += " = " + conds.at(i);
            }
            text += ",";
        }
        if (text.back() == ',') {
            text.pop_back();
        }
        return text + ")]";
    }
};

class type_def: public decl {
private:
    identifier* name;
    generic_type_list* generic_types;

    i64 pointer_depth;
    bool is_const; // mark if the type is const

    number_literal* array_length;
    bool is_array; // mark if the type is array

public:
    type_def(const span& loc):
        decl(ast_type::ast_type_def, loc),
        name(nullptr), generic_types(nullptr),
        pointer_depth(0), is_const(false),
        array_length(nullptr), is_array(false) {}
    ~type_def() override;
    void accept(visitor*) override;
    type_def* clone() const override;

    void set_name(identifier* node) { name = node; }
    auto get_name() { return name; }
    void set_generic_types(generic_type_list* node) { generic_types = node; }
    auto get_generic_types() const { return generic_types; }
    void add_pointer_level() { ++pointer_depth; }
    auto get_pointer_level() { return pointer_depth; }
    void set_constant() { is_const = true; }
    bool is_constant() const { return is_const; }
    void set_array(number_literal* len) { array_length = len; is_array = true; }
    bool get_is_array() const { return is_array; }
    auto get_array_length() const { return array_length; }
};

class generic_type_list: public decl {
private:
    std::vector<type_def*> types;

public:
    generic_type_list(const span& loc):
        decl(ast_type::ast_generic_type_list, loc) {}
    ~generic_type_list() override;
    void accept(visitor*) override;
    generic_type_list* clone() const override;

    void add_type(type_def* node) { types.push_back(node); }
    const auto& get_types() const { return types; }
};

class enum_decl: public decl {
public:
    struct member {
        identifier* name;
        number_literal* value;
    };

private:
    identifier* name;
    std::vector<member> member;

    // conditional compile attribute
    std::vector<cond_compile*> conds;

    // flags
    bool is_public;

public:
    enum_decl(const span& loc):
        decl(ast_type::ast_enum_decl, loc),
        name(nullptr), is_public(false) {}
    ~enum_decl() override;
    void accept(visitor*) override;
    enum_decl* clone() const override;

public:
    void set_name(identifier* node) { name = node; }
    void add_member(identifier* node, number_literal* value) {
        member.push_back({node, value});
    }
    auto get_name() { return name; }
    const auto& get_member() const { return member; }
    void set_public(bool b) { is_public = b; }
    bool is_public_enum() const { return is_public; }

public:
    void add_cond(cond_compile* node) { conds.push_back(node); }
    const auto& get_conds() const { return conds; }
};

class field_pair: public decl {
private:
    identifier* name;
    type_def* type;

public:
    field_pair(const span& loc):
        decl(ast_type::ast_field_pair, loc),
        name(nullptr), type(nullptr) {}
    ~field_pair() override;
    void accept(visitor*) override;
    field_pair* clone() const override;

    void set_name(identifier* node) { name = node; }
    auto get_name() { return name; }
    void set_type(type_def* node) { type = node; }
    auto get_type() { return type; }
};

class struct_decl: public decl {
private:
    std::vector<field_pair*> fields;
    std::string name;
    generic_type_list* generic_types;

    // conditional compile attribute
    std::vector<cond_compile*> conds;

    // flags
    bool is_public;
    bool is_extern;

public:
    struct_decl(const span& loc):
        decl(ast_type::ast_struct_decl, loc),
        generic_types(nullptr), is_public(false), is_extern(false) {}
    ~struct_decl() override;
    void accept(visitor*) override;
    struct_decl* clone() const override;

    void add_field(field_pair* node) { fields.push_back(node); }
    const auto& get_fields() const { return fields; }
    void set_name(const std::string& n) { name = n; }
    const auto& get_name() const { return name; }
    void set_generic_types(generic_type_list* node) { generic_types = node; }
    auto get_generic_types() const { return generic_types; }
    void clear_generic_types() { 
        if (generic_types) {
            delete generic_types;
            generic_types = nullptr;
        }
    }
    void set_public(bool b) { is_public = b; }
    bool is_public_struct() const { return is_public; }
    void set_extern(bool b) { is_extern = b; }
    bool is_extern_struct() const { return is_extern; }

public:
    void add_cond(cond_compile* node) { conds.push_back(node); }
    const auto& get_conds() const { return conds; }
};

class tagged_union_decl: public decl { 
private:
    std::vector<field_pair*> fields;
    std::string name;
    std::string ref_enum_name;

    // conditional compile attribute
    std::vector<cond_compile*> conds;

    // flags
    bool is_public;
    bool is_extern;

public:
    tagged_union_decl(const span& loc):
        decl(ast_type::ast_tagged_union_decl, loc),
        ref_enum_name(""), is_public(false), is_extern(false) {}
    ~tagged_union_decl() override;
    void accept(visitor*) override;
    tagged_union_decl* clone() const override;

    void set_public(bool f) { is_public = f; }
    void set_extern(bool f) { is_extern = f; }

    auto is_public_union() const { return is_public; }
    auto is_extern_union() const { return is_extern; }
    void set_name(const std::string& n) { name = n; }
    const auto& get_name() const { return name; }
    void set_ref_enum_name(const std::string& n) { ref_enum_name = n; }
    const auto& get_ref_enum_name() const { return ref_enum_name; }

    void add_member(field_pair* node) { fields.push_back(node); }
    const auto& get_members() const { return fields; }
    void add_cond(cond_compile* node) { conds.push_back(node); }
    const auto& get_conds() const { return conds; }
};

class param: public decl {
private:
    identifier* name;
    type_def* type;

public:
    param(const span& loc):
        decl(ast_type::ast_param, loc),
        name(nullptr), type(nullptr) {}
    ~param() override;
    void accept(visitor*) override;
    param* clone() const override;

    void set_name(identifier* node) { name = node; }
    auto get_name() { return name; }
    void set_type(type_def* node) { type = node; }
    auto get_type() { return type; }
};

class param_list: public decl {
private:
    std::vector<param*> params;

public:
    param_list(const span& loc):
        decl(ast_type::ast_param_list, loc) {}
    ~param_list() override;
    void accept(visitor*) override;
    param_list* clone() const override;

    void add_param(param* node) { params.push_back(node); }
    auto& get_params() { return params; }
};

class func_decl: public decl {
private:
    std::string name;
    generic_type_list* generic_types;
    param_list* parameters;
    type_def* return_type;
    code_block* block;

    // conditional compile attribute
    std::vector<cond_compile*> conds;

    // flags
    bool is_public;
    bool is_extern;

public:
    func_decl(const span& loc):
        decl(ast_type::ast_func_decl, loc),
        name(""), generic_types(nullptr), parameters(nullptr),
        return_type(nullptr), block(nullptr),
        is_public(false), is_extern(false) {}
    ~func_decl() override;
    void accept(visitor*) override;
    func_decl* clone() const override;

    void set_name(const std::string& n) { name = n; }
    const auto& get_name() const { return name; }
    std::string get_monomorphic_name() const;

public:
    void set_generic_types(generic_type_list* node) { generic_types = node; }
    auto get_generic_types() const { return generic_types; }
    void clear_generic_types() {
        if (generic_types) {
            delete generic_types;
            generic_types = nullptr;
        }
    }

public:
    void set_params(param_list* node) { parameters = node; }
    auto get_params() { return parameters; }
    void set_return_type(type_def* node) { return_type = node; }
    auto get_return_type() { return return_type; }
    void set_code_block(code_block* node) { block = node; }
    auto get_code_block() { return block; }
    void set_public(bool b) { is_public = b; }
    bool is_public_func() const { return is_public; }
    void set_extern(bool b) { is_extern = b; }
    bool is_extern_func() const { return is_extern; }

public:
    void add_cond(cond_compile* node) { conds.push_back(node); }
    const auto& get_conds() const { return conds; }
    bool contain_cond() const;
    cond_compile* get_trivial_cond() const;
    cond_compile* get_non_trivial_cond() const;
    cond_compile* get_is_pointer_cond() const;
    cond_compile* get_is_non_pointer_cond() const;
};

class impl: public decl {
private:
    std::string name;
    generic_type_list* generic_types;
    std::vector<func_decl*> methods;

    // conditional compile attribute
    std::vector<cond_compile*> conds;

public:
    impl(const span& loc, const std::string& n):
        decl(ast_type::ast_impl, loc), name(n),
        generic_types(nullptr) {}
    ~impl() override;
    void accept(visitor*) override;
    impl* clone() const override;

    void set_name(const std::string& n) { name = n; }
    const auto& get_name() const { return name; }
    void set_generic_types(generic_type_list* node) { generic_types = node; }
    auto get_generic_types() const { return generic_types; }
    void add_method(func_decl* node) { methods.push_back(node); }
    const auto& get_methods() const { return methods; }
    void reset_methods(std::vector<func_decl*>& m) {
        methods = m;
    }
    void clear_generic_types() {
        if (generic_types) {
            delete generic_types;
            generic_types = nullptr;
        }
    }

public:
    void add_cond(cond_compile* node) { conds.push_back(node); }
    const auto& get_conds() const { return conds; }
};

}