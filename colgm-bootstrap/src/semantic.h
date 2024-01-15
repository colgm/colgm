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
    void analyse_parameter(param*, colgm_func&);
    void analyse_method_parameter_list(param_list*,
                                       colgm_func&,
                                       const colgm_struct&);
    void analyse_func_parameter_list(param_list*, colgm_func&);
    void analyse_return_type(type_def*, colgm_func&);
    colgm_func analyse_single_method(func_decl*, const colgm_struct&);
    colgm_func analyse_single_func(func_decl*);
    void analyse_functions(root*);
    void analyse_single_impl(impl_struct*);
    void analyse_impls(root*);

private:
    type resolve_binary_operator(binary_operator*);
    type resolve_number_literal(number_literal*);
    type resolve_string_literal(string_literal*);
    type resolve_bool_literal(bool_literal*);
    type resolve_call(call*);
    type resolve_assignment(assignment*);
    type resolve_expression(expr*);
    type resolve_type_def(type_def*);
    void resolve_definition(definition*, const colgm_func&);
    void resolve_if_stmt(if_stmt*, const colgm_func&);
    void resolve_cond_stmt(cond_stmt*, const colgm_func&);
    void resolve_while_stmt(while_stmt*, const colgm_func&);
    void resolve_in_stmt_expr(in_stmt_expr*, const colgm_func&);
    void resolve_ret_stmt(ret_stmt*, const colgm_func&);
    void resolve_statement(stmt*, const colgm_func&);
    void resolve_code_block(code_block*, const colgm_func&);
    void resolve_global_func(func_decl*);
    void resolve_method(func_decl*, const colgm_struct&);
    void resolve_impl(impl_struct*);
    void resolve_function_block(root*);

public:
    const error& analyse(root*);
    void dump();
    const auto& get_context() const { return ctx; }
};

}