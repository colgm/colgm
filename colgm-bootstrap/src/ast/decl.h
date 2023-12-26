#pragma once

#include "ast/ast.h"

#include <cstring>
#include <sstream>
namespace colgm {

class decl: public node {
public:
    decl(ast_type type, const span& loc): node(type, loc) {}
    ~decl() override = default;
    void accept(visitor*) override;
};

class type_def: public decl {
private:
    identifier* name;
    uint64_t pointer_level;

public:
    type_def(const span& loc):
        decl(ast_type::ast_type_def, loc),
        name(nullptr), pointer_level(0) {}
    ~type_def() override;
    void accept(visitor*) override;

    void set_name(identifier* node) { name = node; }
    auto get_name() { return name; }
    void add_pointer_level() { ++pointer_level; }
    auto get_pointer_level() { return pointer_level; }
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

}