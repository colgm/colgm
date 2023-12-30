#pragma once

#include "ast/ast.h"

#include <cstring>
#include <sstream>

namespace colgm {

class expr: public node {
public:
    expr(ast_type type, const span& loc): node(type, loc) {}
    ~expr() override = default;
    void accept(visitor*) override;
};

class binary_operator: public expr {
private:
    expr* left;
    expr* right;

public:
    binary_operator(const span& loc):
        expr(ast_type::ast_binary_operator, loc),
        left(nullptr), right(nullptr) {}
    ~binary_operator() override;
    void accept(visitor*) override;

    void set_left(expr* node) { left = node; }
    auto get_left() const { return left; }
    void set_right(expr* node) { right = node; }
    auto get_right() const { return right; }
};

class identifier: public expr {
private:
    std::string name;

public:
    identifier(const span& loc, const std::string& id_name):
        expr(ast_type::ast_identifier, loc), name(id_name) {}
    ~identifier() override = default;
    void accept(visitor*) override;

    const auto& get_name() const { return name; }
};

class number_literal: public expr {
private:
    std::string literal;

public:
    number_literal(const span& loc, const std::string& number):
        expr(ast_type::ast_number_literal, loc), literal(number) {}
    ~number_literal() override = default;
    void accept(visitor*) override;

    const auto& get_number() const { return literal; }
};

class string_literal: public expr {
private:
    std::string literal;

public:
    string_literal(const span& loc, const std::string& str):
        expr(ast_type::ast_string_literal, loc), literal(str) {}
    ~string_literal() override = default;
    void accept(visitor*) override;

    const auto& get_string() const { return literal; }
};

class call_index: public expr {
private:
    expr* index;

public:
    call_index(const span& loc):
        expr(ast_type::ast_call_index, loc), index(nullptr) {}
    ~call_index() override;
    void accept(visitor*) override;

    void set_index(expr* node) { index = node; }
    auto get_index() const { return index; }
};

class call_func_args: public expr {
private:
    std::vector<expr*> args;

public:
    call_func_args(const span& loc):
        expr(ast_type::ast_call_func_args, loc) {}
    ~call_func_args() override;
    void accept(visitor*) override;

    void add_arg(expr* node) { args.push_back(node); }
    const auto& get_args() const { return args; }
};

class call_field: public expr {
private:
    std::string name;
    call_func_args* func_call_args;

public:
    call_field(const span& loc, const std::string& f):
        expr(ast_type::ast_call_field, loc), name(f), func_call_args(nullptr) {}
    ~call_field() override;
    void accept(visitor*) override;

    const auto& get_name() const { return name; }
    auto get_args() const { return func_call_args; }
};

class call: public expr {
private:
    identifier* head;
    std::vector<expr*> chain;

public:
    call(const span& loc):
        expr(ast_type::ast_call, loc), head(nullptr) {}
    ~call() override;
    void accept(visitor*) override;

    void set_head(identifier* node) { head = node; }
    auto get_head() const { return head; }
    void add_chain(expr* node) { chain.push_back(node); }
    const auto& get_chain() const { return chain; }
};

}