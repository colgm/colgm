#pragma once

#include "ast/ast.h"
#include "ast/expr.h"

#include <vector>

namespace colgm::ast {

class stmt: public node {
public:
    stmt(ast_type type, const span& loc): node(type, loc) {}
    ~stmt() override = default;
    void accept(visitor*) override;  
};

class use_stmt: public stmt {
private:
    std::vector<identifier*> module_path;
    std::vector<identifier*> import_symbol;

public:
    use_stmt(const span& loc):
        stmt(ast_type::ast_use_stmt, loc) {}
    ~use_stmt() override;
    void accept(visitor*) override;

    void add_path(identifier* node) { module_path.push_back(node); }
    void add_import_symbol(identifier* node) { import_symbol.push_back(node); }

    auto& get_module_path() { return module_path; }
    auto& get_import_symbol() { return import_symbol; }
};

class definition: public stmt {
private:
    std::string name;
    type_def* type;
    expr* init_value;

public:
    definition(const span& loc, const std::string& n):
        stmt(ast_type::ast_definition, loc),
        name(n), type(nullptr), init_value(nullptr) {}
    ~definition() override;
    void accept(visitor*) override;

    const auto& get_name() const { return name; }
    void set_type(type_def* node) { type = node; }
    auto get_type() const { return type; }
    void set_init_value(expr* node) { init_value = node; }
    auto get_init_value() const { return init_value; }
};

class cond_stmt: public stmt {
private:
    std::vector<if_stmt*> stmts;

public:
    cond_stmt(const span& loc):
        stmt(ast_type::ast_cond_stmt, loc) {}
    ~cond_stmt() override;
    void accept(visitor*) override;

    void add_stmt(if_stmt* node) { stmts.push_back(node); }
    const auto& get_stmts() const { return stmts; }
};

class if_stmt: public stmt {
private:
    expr* condition;
    code_block* block;

public:
    if_stmt(const span& loc):
        stmt(ast_type::ast_if_stmt, loc),
        condition(nullptr), block(nullptr) {}
    ~if_stmt() override;
    void accept(visitor*) override;

    void set_condition(expr* node) { condition = node; }
    auto get_condition() const { return condition; }
    void set_block(code_block* node) { block = node; }
    auto get_block() const { return block; }
};

class match_case: public stmt {
private:
    expr* value;
    code_block* block;

public:
    match_case(const span& loc):
        stmt(ast_type::ast_match_case, loc),
        value(nullptr), block(nullptr) {}
    ~match_case() override;
    void accept(visitor*) override;

    void set_value(expr* node) { value = node; }
    auto get_value() const { return value; }
    void set_block(code_block* node){ block = node; }
    auto get_block() const { return block; }
};

class match_stmt: public stmt {
private:
    expr* value;
    std::vector<match_case*> cases;

public:
    match_stmt(const span& loc):
        stmt(ast_type::ast_match_stmt, loc),
        value(nullptr) {}
    ~match_stmt() override;
    void accept(visitor*) override;

    void set_value(expr* node) { value = node; }
    auto get_value() const { return value; }
    void add_case(match_case* node) {cases.push_back(node); }
    const auto& get_cases() const { return cases; }
};

class while_stmt: public stmt {
private:
    expr* condition;
    code_block* block;

public:
    while_stmt(const span& loc):
        stmt(ast_type::ast_while_stmt, loc),
        condition(nullptr), block(nullptr) {}
    ~while_stmt() override;
    void accept(visitor*) override;

    void set_condition(expr* node) { condition = node; }
    auto get_condition() const { return condition; }
    void set_block(code_block* node) { block = node; }
    auto get_block() const { return block; }
};

class for_stmt: public stmt {
private:
    definition* init;
    expr* condition;
    expr* update;
    code_block* block;

public:
    for_stmt(const span& loc):
        stmt(ast_type::ast_for_stmt, loc),
        condition(nullptr), update(nullptr), block(nullptr) {}
    ~for_stmt() override;
    void accept(visitor*) override;

    void set_init(definition* node) { init = node; }
    auto get_init() const { return init; }
    void set_condition(expr* node) { condition = node; }
    auto get_condition() const { return condition; }
    void set_update(expr* node) { update = node; }
    auto get_update() const { return update; }
    void set_block(code_block* node) { block = node; }
    auto get_block() const { return block; }
};

class in_stmt_expr: public stmt {
private:
    expr* calculation;

public:
    in_stmt_expr(const span& loc):
        stmt(ast_type::ast_in_stmt_expr, loc),
        calculation(nullptr) {}
    ~in_stmt_expr() override;
    void accept(visitor*) override;

    void set_expr(expr* node) { calculation = node; }
    auto get_expr() const { return calculation; }
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

class continue_stmt: public stmt {
public:
    continue_stmt(const span& loc):
        stmt(ast_type::ast_continue_stmt, loc) {}
    ~continue_stmt() override = default;
    void accept(visitor*) override;
};

class break_stmt: public stmt {
public:
    break_stmt(const span& loc):
        stmt(ast_type::ast_break_stmt, loc) {}
    ~break_stmt() override = default;
    void accept(visitor*) override;
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