#pragma once

#include "ast/ast.h"

namespace colgm {

class expr: public ast_node {
public:
    expr(ast_type type, const span& loc): ast_node(type, loc) {}
    virtual ~expr() = default;
    virtual void accept(visitor*) override;
};

}