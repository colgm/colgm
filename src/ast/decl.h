#pragma once

#include "ast/ast.h"
namespace colgm {

class decl: public node {
public:
    decl(ast_type type, const span& loc): node(type, loc) {}
    virtual ~decl() override = default;
    virtual void accept(visitor*) override;
};

}