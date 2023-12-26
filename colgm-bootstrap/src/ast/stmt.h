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

class code_block: public stmt {
public:
    code_block(const span& loc): stmt(ast_type::ast_code_block, loc) {}
    void accept(visitor*) override;
};

}