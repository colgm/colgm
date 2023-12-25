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
    virtual void accept(visitor*);

    const auto& get_name() const { return name; }
};

}