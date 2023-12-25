#pragma once

#include "ast/ast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"

namespace colgm {

class visitor {
public:
    virtual bool visit_node(node*);
    virtual bool visit_root(root*);
    virtual bool visit_decl(decl*);
    virtual bool visit_expr(expr*);
    virtual bool visit_identifier(identifier*);
    virtual bool visit_stmt(stmt*);
    virtual bool visit_use_stmt(use_stmt*);
    virtual bool visit_specified_use(specified_use*);
};

}