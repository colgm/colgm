#pragma once

#include "colgm.h"
#include "ast/ast.h"

#include <cstring>
#include <sstream>

namespace colgm::ast {

class decl: public node {
public:
    decl(ast_type type, const span& loc): node(type, loc) {}
    ~decl() override = default;
    void accept(visitor*) override;
};

class type_def: public decl {
private:
    identifier* name;
    i64 pointer_depth;

public:
    type_def(const span& loc):
        decl(ast_type::ast_type_def, loc),
        name(nullptr), pointer_depth(0) {}
    ~type_def() override;
    void accept(visitor*) override;

    void set_name(identifier* node) { name = node; }
    auto get_name() { return name; }
    void add_pointer_level() { ++pointer_depth; }
    auto get_pointer_level() { return pointer_depth; }
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

public:
    enum_decl(const span& loc):
        decl(ast_type::ast_enum_decl, loc),
        name(nullptr) {}
    ~enum_decl() override;
    void accept(visitor*) override;

public:
    void set_name(identifier* node) { name = node; }
    void add_member(identifier* node, number_literal* value) {
        member.push_back({node, value});
    }
    auto get_name() { return name; }
    const auto& get_member() const { return member; }
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

    void set_name(identifier* node) { name = node; }
    auto get_name() { return name; }
    void set_type(type_def* node) { type = node; }
    auto get_type() { return type; }
};

class struct_decl: public decl {
private:
    std::vector<struct_field*> fields;
    std::string name;

public:
    struct_decl(const span& loc):
        decl(ast_type::ast_struct_decl, loc) {}
    ~struct_decl() override;
    void accept(visitor*) override;

    void add_field(struct_field* node) { fields.push_back(node); }
    const auto& get_fields() const { return fields; }
    void set_name(const std::string& n) { name = n; }
    const auto& get_name() const { return name; }
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

    void add_param(param* node) { params.push_back(node); }
    auto& get_params() { return params; }
};

class func_decl: public decl {
private:
    std::string name;
    param_list* parameters;
    type_def* return_type;
    code_block* block;

public:
    func_decl(const span& loc):
        decl(ast_type::ast_func_decl, loc),
        name(""), parameters(nullptr),
        return_type(nullptr), block(nullptr) {}
    ~func_decl() override;
    void accept(visitor*) override;

    void set_name(const std::string& n) { name = n; }
    const auto& get_name() const { return name; }
    void set_params(param_list* node) { parameters = node; }
    auto get_params() { return parameters; }
    void set_return_type(type_def* node) { return_type = node; }
    auto get_return_type() { return return_type; }
    void set_code_block(code_block* node) { block = node; }
    auto get_code_block() { return block; }
};

class impl_struct: public decl {
private:
    std::string struct_name;
    std::vector<func_decl*> methods;

public:
    impl_struct(const span& loc, const std::string& sn):
        decl(ast_type::ast_impl, loc), struct_name(sn) {}
    ~impl_struct() override;
    void accept(visitor*) override;

    const auto& get_struct_name() const { return struct_name; }
    void add_method(func_decl* node) { methods.push_back(node); }
    const auto& get_methods() const { return methods; }
};

}