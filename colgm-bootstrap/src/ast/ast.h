#pragma once

#include "err.h"

#include <vector>

namespace colgm {

enum class ast_type {
    ast_null = 0,
    ast_root,
    ast_identifier,
    ast_type_def,
    ast_param,
    ast_param_list,
    ast_func_decl,
    ast_code_block
};

class visitor;

class decl;
class type_def;
class param;
class param_list;
class func_decl;
class expr;
class identifier;
class stmt;
class code_block;

class node {
private:
    ast_type node_type;
    span location;

public:
    node(ast_type type, const span& loc): node_type(type), location(loc) {}
    virtual ~node() = default;
    virtual void accept(visitor*) = 0;

    const auto& get_location() const { return location; }
};

class root: public node {
private:
    std::vector<decl*> declarations;

public:
    root(const span& loc): node(ast_type::ast_root, loc) {}
    ~root() override;
    void accept(visitor*) override;

    void add_decl(decl* node) { declarations.push_back(node); }
    const auto& get_decls() const { return declarations; }
};

}