#pragma once
#include "ast/ast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "code/ir.h"
#include "ast/visitor.h"
#include "sema/symbol.h"

#include <unordered_map>
#include <cstring>
#include <sstream>
#include <vector>

namespace colgm {

class semantic {
private:
    error err;
    semantic_context ctx;

private:
    void analyse_single_struct(struct_decl*);
    void analyse_structs(root*);
    colgm_func analyse_single_func(func_decl*);
    void analyse_functions(root*);
    void analyse_single_impl(impl_struct*);
    void analyse_impls(root*);

private:
    type resolve_expression(expr*);
    type resolve_type_def(type_def*);
    void resolve_parameter(param*);
    void resolve_parameter_list(param_list*);
    void resolve_func(func_decl*);
    void resolve_impl(impl_struct*);
    void resolve_function_block(root*);

public:
    const error& analyse(root*);
    void dump();
    const auto& get_context() const { return ctx; }
};

}