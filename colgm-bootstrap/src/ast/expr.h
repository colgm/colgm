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

class call_field: public expr {
private:
    std::string name;
    expr* func_call_args;

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