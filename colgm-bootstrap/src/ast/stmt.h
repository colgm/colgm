#pragma once

#include "ast/ast.h"
#include "ast/expr.h"

#include <vector>

namespace colgm {

class stmt: public node {
public:
    stmt(ast_type type, const span& loc): node(type, loc) {}
    ~stmt() override = default;
    void accept(visitor*) override;  
};

class ret_stmt: public stmt {
private:
    expr* value;

public:
    ret_stmt(const span& loc):
        stmt(ast_type::ast_ret_stmt, loc), value(nullptr) {}
    ~ret_stmt() override;
    void accept(visitor*) override;

    void set_value(expr* node) { value = node; }
    auto get_value() const { return value; }
};

class code_block: public stmt {
private:
    std::vector<stmt*> statements;

public:
    code_block(const span& loc): stmt(ast_type::ast_code_block, loc) {}
    ~code_block() override;
    void accept(visitor*) override;

    void add_stmt(stmt* node) { statements.push_back(node); }
    const auto& get_stmts() const { return statements; }
};

}