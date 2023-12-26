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
    virtual bool visit_type_def(type_def*);
    virtual bool visit_param(param*);
    virtual bool visit_param_list(param_list*);
    virtual bool visit_func_decl(func_decl*);
    virtual bool visit_expr(expr*);
    virtual bool visit_identifier(identifier*);
    virtual bool visit_stmt(stmt*);
    virtual bool visit_code_block(code_block*);
};

}