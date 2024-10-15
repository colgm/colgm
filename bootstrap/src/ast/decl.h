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

class type_def: public decl {
private:
    identifier* name;
    generic_type_list* generic_types;
    i64 pointer_depth;
    bool is_const;

public:
    type_def(const span& loc):
        decl(ast_type::ast_type_def, loc),
        name(nullptr), generic_types(nullptr),
        pointer_depth(0), is_const(false) {}
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
};

class struct_field: public decl {
private:
    identifier* name;
    type_def* type;

public:
    struct_field(const span& loc):
        decl(ast_type::ast_struct_field, loc),
        name(nullptr), type(nullptr) {}
    ~struct_field() override;
    void accept(visitor*) override;
    struct_field* clone() const override;

    void set_name(identifier* node) { name = node; }
    auto get_name() { return name; }
    void set_type(type_def* node) { type = node; }
    auto get_type() { return type; }
};

class struct_decl: public decl {
private:
    std::vector<struct_field*> fields;
    std::string name;
    generic_type_list* generic_types;
    bool is_public;
    bool is_extern;

public:
    struct_decl(const span& loc):
        decl(ast_type::ast_struct_decl, loc),
        generic_types(nullptr), is_public(false), is_extern(false) {}
    ~struct_decl() override;
    void accept(visitor*) override;
    struct_decl* clone() const override;

    void add_field(struct_field* node) { fields.push_back(node); }
    const auto& get_fields() const { return fields; }
    void set_name(const std::string& n) { name = n; }
    const auto& get_name() const { return name; }
    void set_generic_types(generic_type_list* node) { generic_types = node; }
    auto get_generic_types() const { return generic_types; }
    void set_public(bool b) { is_public = b; }
    bool is_public_struct() const { return is_public; }
    void set_extern(bool b) { is_extern = b; }
    bool is_extern_struct() const { return is_extern; }
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
    void set_generic_types(generic_type_list* node) { generic_types = node; }
    auto get_generic_types() const { return generic_types; }
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
    void clear_generic_types() {
        if (generic_types) {
            delete generic_types;
            generic_types = nullptr;
        }
    }
};

class impl_struct: public decl {
private:
    std::string struct_name;
    generic_type_list* generic_types;
    std::vector<func_decl*> methods;

public:
    impl_struct(const span& loc, const std::string& sn):
        decl(ast_type::ast_impl, loc), struct_name(sn),
        generic_types(nullptr) {}
    ~impl_struct() override;
    void accept(visitor*) override;
    impl_struct* clone() const override;

    const auto& get_struct_name() const { return struct_name; }
    void set_generic_types(generic_type_list* node) { generic_types = node; }
    auto get_generic_types() const { return generic_types; }
    void add_method(func_decl* node) { methods.push_back(node); }
    const auto& get_methods() const { return methods; }
    void clear_generic_types() {
        if (generic_types) {
            delete generic_types;
            generic_types = nullptr;
        }
    }
};

}