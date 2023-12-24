#pragma once

#include "ast/ast.h"

namespace colgm {

class stmt: public ast_node {
public:
    stmt(ast_type type, const span& loc): ast_node(type, loc) {}
    virtual ~stmt() = default;
    virtual void accept(visitor*) override;
    
};

}