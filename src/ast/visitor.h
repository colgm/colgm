#pragma once

#include "ast/ast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"

namespace colgm {

class visitor {
public:
    virtual bool visit_ast_node(ast_node*);
    virtual bool visit_decl(decl*);
    virtual bool visit_expr(expr*);
    virtual bool visit_stmt(stmt*);
};

}