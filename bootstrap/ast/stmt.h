#pragma once

#include "ast/ast.h"
#include "ast/expr.h"

#include <vector>
#include <cassert>

namespace colgm::ast {

class stmt: public node {
public:
    stmt(ast_type type, const span& loc): node(type, loc) {}
    ~stmt() override = default;
    void accept(visitor*) override;
    stmt* clone() const override {
        assert(false && "not implemented: class stmt");
        return nullptr;
    }
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
    use_stmt* clone() const override;

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
    definition* clone() const override;

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
    cond_stmt* clone() const override;

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
    if_stmt* clone() const override;

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
    match_case* clone() const override;

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
    match_stmt* clone() const override;

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
    while_stmt* clone() const override;

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
        init(nullptr), condition(nullptr),
        update(nullptr), block(nullptr) {}
    ~for_stmt() override;
    void accept(visitor*) override;
    for_stmt* clone() const override;

    void set_init(definition* node) { init = node; }
    auto get_init() const { return init; }
    void set_condition(expr* node) { condition = node; }
    auto get_condition() const { return condition; }
    void set_update(expr* node) { update = node; }
    auto get_update() const { return update; }
    void set_block(code_block* node) { block = node; }
    auto get_block() const { return block; }
};

class forindex: public stmt {
private:
    identifier* variable;
    call* container;
    code_block* body;

    definition* lowered_init;
    expr* lowered_condition;
    expr* lowered_update;

public:
    forindex(const span& loc):
        stmt(ast_type::ast_forindex, loc),
        variable(nullptr), container(nullptr),
        body(nullptr), lowered_init(nullptr),
        lowered_condition(nullptr), lowered_update(nullptr) {}
    ~forindex() override;
    void accept(visitor*) override;
    forindex* clone() const override;

    void set_variable(identifier* node) { variable = node; }
    auto get_variable() const { return variable; }
    void set_container(call* node) { container = node; }
    auto get_container() const { return container; }
    void set_body(code_block* node) { body = node; }
    auto get_body() const { return body; }
    void set_lowered_init(definition* node) { lowered_init = node; }
    auto get_lowered_init() const { return lowered_init; }
    void set_lowered_condition(expr* node) { lowered_condition = node; }
    auto get_lowered_condition() const { return lowered_condition; }
    void set_lowered_update(expr* node) { lowered_update = node; }
    auto get_lowered_update() const { return lowered_update; }
};

class foreach: public stmt {
private:
    identifier* variable;
    call* container;
    code_block* body;

    definition* lowered_init;
    expr* lowered_condition;
    expr* lowered_update;

public:
    foreach(const span& loc):
        stmt(ast_type::ast_foreach, loc),
        variable(nullptr), container(nullptr),
        body(nullptr), lowered_init(nullptr),
        lowered_condition(nullptr), lowered_update(nullptr) {}
    ~foreach() override;
    void accept(visitor*) override;
    foreach* clone() const override;

    void set_variable(identifier* node) { variable = node; }
    auto get_variable() const { return variable; }
    void set_container(call* node) { container = node; }
    auto get_container() const { return container; }
    void set_body(code_block* node) { body = node; }
    auto get_body() const { return body; }
    void set_lowered_init(definition* node) { lowered_init = node; }
    auto get_lowered_init() const { return lowered_init; }
    void set_lowered_condition(expr* node) { lowered_condition = node; }
    auto get_lowered_condition() const { return lowered_condition; }
    void set_lowered_update(expr* node) { lowered_update = node; }
    auto get_lowered_update() const { return lowered_update; }
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
    in_stmt_expr* clone() const override {
        auto ret = new in_stmt_expr(location);
        ret->calculation = calculation->clone();
        return ret;
    }

    void set_expr(expr* node) { calculation = node; }
    auto get_expr() const { return calculation; }
};

class defer_stmt: public stmt {
private:
    code_block* block;

public:
    defer_stmt(const span& loc):
        stmt(ast_type::ast_defer_stmt, loc),
        block(nullptr) {}
    ~defer_stmt() override;
    void accept(visitor*) override;
    defer_stmt* clone() const override;

    void set_block(code_block* node) { block = node; }
    auto get_block() const { return block; }
};

class ret_stmt: public stmt {
private:
    expr* value;
    bool return_ref_type;

public:
    ret_stmt(const span& loc):
        stmt(ast_type::ast_ret_stmt, loc),
        value(nullptr), return_ref_type(false) {}
    ~ret_stmt() override;
    void accept(visitor*) override;
    ret_stmt* clone() const override {
        auto ret = new ret_stmt(*this);
        if (value) {
            ret->value = value->clone();
        }
        ret->return_ref_type = return_ref_type;
        return ret;
    }

    void set_value(expr* node) { value = node; }
    auto get_value() const { return value; }
    void set_return_ref_type(bool val) { return_ref_type = val; }
    bool get_return_ref_type() const { return return_ref_type; }
};

class continue_stmt: public stmt {
public:
    continue_stmt(const span& loc):
        stmt(ast_type::ast_continue_stmt, loc) {}
    ~continue_stmt() override = default;
    void accept(visitor*) override;
    continue_stmt* clone() const override {
        return new continue_stmt(location);
    }
};

class break_stmt: public stmt {
public:
    break_stmt(const span& loc):
        stmt(ast_type::ast_break_stmt, loc) {}
    ~break_stmt() override = default;
    void accept(visitor*) override;
    break_stmt* clone() const override {
        return new break_stmt(location);
    }
};

class code_block: public stmt {
private:
    std::vector<stmt*> statements;

public:
    code_block(const span& loc): stmt(ast_type::ast_code_block, loc) {}
    ~code_block() override;
    void accept(visitor*) override;
    code_block* clone() const override;

    void add_stmt(stmt* node) { statements.push_back(node); }
    const auto& get_stmts() const { return statements; }
    void reset_stmt_with(const std::vector<stmt*>& v) {
        statements = v;
    }
    // last statement is not return/continue/break
    bool back_not_block_exit() const {
        if (statements.empty()) {
            return true;
        }
        auto back = statements.back();
        return !back->is(ast_type::ast_ret_stmt) &&
               !back->is(ast_type::ast_continue_stmt) &&
               !back->is(ast_type::ast_break_stmt);
    }
};

}