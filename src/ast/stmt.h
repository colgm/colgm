#pragma once

#include "ast/ast.h"
#include "ast/expr.h"

#include <vector>

namespace colgm {

class stmt: public node {
public:
    stmt(ast_type type, const span& loc): node(type, loc) {}
    virtual ~stmt() override = default;
    virtual void accept(visitor*) override;  
};

class use_stmt: public stmt {
public:
    enum class use_kind {
        use_all,
        use_specify,
        use_single
    };

private:
    std::vector<identifier*> path;
    use_kind kind;
    identifier* single_use;
    std::vector<identifier*> specified_use;

public:
    use_stmt(const span& loc):
        stmt(ast_type::ast_use_stmt, loc),
        kind(use_kind::use_all), single_use(nullptr) {}
    ~use_stmt() override;
    void accept(visitor*) override;

    void add_path(identifier* node) { path.push_back(node); }
    void set_use_kind(use_kind k) { kind = k; }
    void set_single_use(identifier* node) { single_use = node; }
    void add_specified_use(identifier* node) { specified_use.push_back(node); }

    const auto get_use_kind() const { return kind; }
    const auto& get_path() const { return path; }
    auto get_single_use() const { return single_use; }
    const auto& get_specified_use() const { return specified_use; }
};

}