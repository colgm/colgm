#pragma once
#include "ast/ast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "code/sir.h"
#include "ast/visitor.h"
#include "sema/symbol.h"
#include "package/package.h"
#include "sema/context.h"

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
    void report(node* n, const std::string& info) {
        err.err("sema", n->get_location(), info);
    }
    void warning(node* n, const std::string& info) {
        err.warn("sema", n->get_location(), info);
    }
    void unimplemented(node* n) {
        err.err("sema", n->get_location(), "unimplemented, please report a bug.");
    }
    void unreachable(node* n) {
        err.err("sema", n->get_location(), "unreachable, please report a bug.");
    }

private:
    bool flag_enable_type_warning = false;

public:
    void enable_type_warning() {
        flag_enable_type_warning = true;
    }

private:
    colgm_func builtin_struct_size(const span&);
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
    type struct_static_method_infer(const std::string&,
                                    const std::string&,
                                    const std::string&);
    type struct_method_infer(const std::string&,
                             const std::string&,
                             const std::string&);

private:
    type resolve_logical_operator(binary_operator*);
    type resolve_comparison_operator(binary_operator*);
    type resolve_arithmetic_operator(binary_operator*);
    type resolve_binary_operator(binary_operator*);
    type resolve_number_literal(number_literal*);
    type resolve_string_literal(string_literal*);
    type resolve_bool_literal(bool_literal*);
    type resolve_identifier(identifier*);
    type resolve_call_field(const type&, call_field*);
    void check_static_call_args(const colgm_func&, call_func_args*);
    void check_method_call_args(const colgm_func&, const type&, call_func_args*);
    type resolve_call_func_args(const type&, call_func_args*);
    type resolve_call_index(const type&, call_index*);
    type resolve_call_path(const type&, call_path*);
    type resolve_ptr_call_field(const type&, ptr_call_field*);
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

private:
    void import_global_symbol(node* n, const std::string&, const symbol_info&);
    void resolve_single_use(use_stmt*);
    void resolve_use_stmt(root*);

public:
    const error& analyse(root*);
    void dump();
    const auto& get_context() const { return ctx; }
};

}