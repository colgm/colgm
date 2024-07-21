#pragma once

#include "ast/ast.h"

#include <cstring>
#include <sstream>

namespace colgm::ast {

class expr: public node {
public:
    expr(ast_type type, const span& loc): node(type, loc) {}
    ~expr() override = default;
    void accept(visitor*) override;
};

class null: public expr {
public:
    null(): expr(ast_type::ast_null, span::null()) {}
    ~null() override = default;
    void accept(visitor*) override { return; }
};

class unary_operator: public expr {
public:
    enum class kind {
        neg,
        bnot
    };

private:
    kind opr;
    expr* value;

public:
    unary_operator(const span& loc):
        expr(ast_type::ast_unary_operator, loc), value(nullptr) {}
    ~unary_operator() override;
    void accept(visitor*) override;

    void set_opr(kind t) { opr = t; }
    auto get_opr() const { return opr; }
    void set_value(expr* node) { value = node; }
    auto get_value() const { return value; }
};

class binary_operator: public expr {
public:
    enum class kind {
        add,
        sub,
        mult,
        div,
        rem,
        cmpeq,
        cmpneq,
        less,
        leq,
        grt,
        geq,
        cmpand,
        cmpor,
        band,
        bor,
        bxor,
    };

private:
    kind opr;
    expr* left;
    expr* right;

public:
    binary_operator(const span& loc):
        expr(ast_type::ast_binary_operator, loc),
        left(nullptr), right(nullptr) {}
    ~binary_operator() override;
    void accept(visitor*) override;

    void set_opr(kind t) { opr = t; }
    auto get_opr() const { return opr; }
    void set_left(expr* node) { left = node; }
    auto get_left() const { return left; }
    void set_right(expr* node) { right = node; }
    auto get_right() const { return right; }
};

class type_convert: public expr {
private:
    expr* source;
    type_def* target;

public:
    type_convert(const span& loc):
        expr(ast_type::ast_type_convert, loc),
        source(nullptr), target(nullptr) {}
    ~type_convert() override;
    void accept(visitor*) override;

public:
    void set_source(expr* s) { source = s; }
    void set_target(type_def* t) { target = t; }
    auto get_source() const { return source; }
    auto get_target() const { return target; }
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

class nil_literal: public expr {
public:
    nil_literal(const span& loc):
        expr(ast_type::ast_nil_literal, loc) {}
    ~nil_literal() override = default;
    void accept(visitor*) override;
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
    // used to set 0o____ & 0x____ number to literal that llvm could recognize
    void reset_number(const std::string& number) { literal = number; }
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

class char_literal: public expr {
private:
    char literal;

public:
    char_literal(const span& loc, const char c):
        expr(ast_type::ast_char_literal, loc), literal(c) {}
    ~char_literal() override = default;
    void accept(visitor*) override;

    auto get_char() const { return literal; }
};

class bool_literal: public expr {
private:
    bool flag;

public:
    bool_literal(const span& loc, bool f):
        expr(ast_type::ast_bool_literal, loc), flag(f) {}
    ~bool_literal() override = default;
    void accept(visitor*) override;

    auto get_flag() const { return flag; }
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

public:
    call_field(const span& loc, const std::string& f):
        expr(ast_type::ast_call_field, loc), name(f) {}
    ~call_field() override = default;
    void accept(visitor*) override;

    const auto& get_name() const { return name; }
};

class ptr_call_field: public expr {
private:
    std::string name;

public:
    ptr_call_field(const span& loc, const std::string& f):
        expr(ast_type::ast_ptr_call_field, loc), name(f) {}
    ~ptr_call_field() override = default;
    void accept(visitor*) override;

    const auto& get_name() const { return name; }
};

class call_path: public expr {
private:
    std::string name;

public:
    call_path(const span& loc, const std::string& f):
        expr(ast_type::ast_call_path, loc), name(f) {}
    ~call_path() override = default;
    void accept(visitor*) override;

    const auto& get_name() const { return name; }
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

class assignment: public expr {
public:
    enum class kind {
        eq,
        addeq,
        subeq,
        multeq,
        diveq,
        remeq,
        andeq,
        xoreq,
        oreq
    };

private:
    kind type;
    call* left;
    expr* right;

public:
    assignment(const span& loc):
        expr(ast_type::ast_assignment, loc),
        left(nullptr), right(nullptr) {}
    ~assignment() override;
    void accept(visitor*) override;

    void set_type(kind t) { type = t; }
    auto get_type() const { return type; }
    void set_left(call* node) { left = node; }
    auto get_left() const { return left; }
    void set_right(expr* node) { right = node; }
    auto get_right() const { return right; }
};

}