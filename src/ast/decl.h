#pragma once

#include "ast/ast.h"
namespace colgm {

class decl: public ast_node {
public:
    decl(ast_type type, const span& loc): ast_node(type, loc) {}
    virtual ~decl() = default;
    virtual void accept(visitor*) override;
};

}