#pragma once

#include "err.h"

#include <vector>

namespace colgm {

enum class ast_type {
    ast_null = 0,
    ast_root,
    ast_identifier,
    ast_use_stmt,
    ast_specified_use,
};

class visitor;

class decl;
class expr;
class stmt;
class use_stmt;
class specified_use;

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
    std::vector<use_stmt*> use_statements;
    std::vector<decl*> declarations;

public:
    root(const span& loc): node(ast_type::ast_root, loc) {}
    ~root() override;
    void accept(visitor*) override;

    void add_use_statement(use_stmt* node) { use_statements.push_back(node); }
    const auto& get_use_statements() const { return use_statements; }
    const auto& get_declarations() const { return declarations; }
};

}