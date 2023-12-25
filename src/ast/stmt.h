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

class use_stmt: public stmt {
public:
    enum class use_kind {
        use_all,
        use_specify
    };

private:
    std::vector<identifier*> path;
    use_kind kind;
    specified_use* specified;

public:
    use_stmt(const span& loc):
        stmt(ast_type::ast_use_stmt, loc),
        kind(use_kind::use_all), specified(nullptr) {}
    ~use_stmt() override;
    void accept(visitor*) override;

    void add_path(identifier* node) { path.push_back(node); }
    void set_use_kind(use_kind k) { kind = k; }
    void set_specified(specified_use* node) { specified = node; }

    const auto get_use_kind() const { return kind; }
    auto& get_path() { return path; }
    auto get_specified() const { return specified; }
};

class specified_use: public stmt {
private:
    std::vector<identifier*> symbols;

public:
    specified_use(const span& loc):
        stmt(ast_type::ast_specified_use, loc) {}
    ~specified_use() override;
    void accept(visitor*) override;

    void add_symbol(identifier* node) { symbols.push_back(node); }
    const auto& get_symbols() const { return symbols; }
};

}