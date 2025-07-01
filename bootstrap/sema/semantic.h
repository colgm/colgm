#pragma once
#include "ast/ast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "sir/sir.h"
#include "ast/visitor.h"
#include "sema/type.h"
#include "sema/func.h"
#include "sema/struct.h"
#include "sema/tagged_union.h"
#include "package/package.h"
#include "sema/context.h"
#include "sema/reporter.h"
#include "sema/type_resolver.h"

#include <unordered_map>
#include <cstring>
#include <sstream>
#include <vector>

namespace colgm {

using namespace ast;

class semantic {
private:
    error& err;
    sema_context ctx;
    reporter rp;
    type_resolver tr;
    i64 in_loop_level = 0;
    std::string impl_struct_name;

private:
    void report_unreachable_statements(code_block*);
    void report_top_level_block_has_no_return(code_block*, const colgm_func&);

private:
    bool number_literal_can_be_converted(node*, const type&);
    bool unary_number_can_be_converted(node*, const type&);
    bool nil_can_be_converted(node*, const type&);
    bool check_can_be_converted(node*, const type&);

private:
    type struct_static_method_infer(const type&, const std::string&);
    type struct_method_infer(const type&, const std::string&);

private:
    type resolve_logical_operator(binary_operator*);
    type resolve_comparison_operator(binary_operator*);
    type resolve_arithmetic_operator(binary_operator*);
    type resolve_bitwise_operator(binary_operator*);
    type resolve_binary_operator(binary_operator*);
    type resolve_unary_neg(unary_operator*);
    type resolve_unary_bnot(unary_operator*);
    type resolve_unary_lnot(unary_operator*);
    type resolve_unary_operator(unary_operator*);
    type resolve_type_convert(type_convert*);
    type resolve_nil_literal(nil_literal*);
    type resolve_number_literal(number_literal*);
    type resolve_string_literal(string_literal*);
    type resolve_char_literal(char_literal*);
    type resolve_bool_literal(bool_literal*);
    type resolve_array_list(array_list*);
    type resolve_identifier(identifier*);
    void check_pub_method(node*, const std::string&, const colgm_struct&);
    void check_pub_method(node*, const std::string&, const colgm_tagged_union&);
    void check_pub_static_method(node*, const std::string&, const colgm_struct&);
    void check_pub_static_method(node*, const std::string&, const colgm_tagged_union&);
    type resolve_get_field(const type&, get_field*);
    type resolve_struct_get_field(const colgm_struct&, const type&, get_field*);
    type resolve_tagged_union_get_field(const colgm_tagged_union&, const type&, get_field*);
    void check_static_call_args(const colgm_func&, call_func_args*);
    void check_method_call_args(const colgm_func&, call_func_args*);
    type resolve_call_id(call_id*);
    type resolve_call_func_args(const type&, call_func_args*);
    type resolve_call_index(const type&, call_index*);
    type resolve_initializer(const type&, initializer*);
    type resolve_call_path(const type&, call_path*);
    type resolve_ptr_get_field(const type&, ptr_get_field*);
    type resolve_struct_ptr_get_field(const colgm_struct&, const type&, ptr_get_field*);
    type resolve_tagged_union_ptr_get_field(const colgm_tagged_union&, const type&, ptr_get_field*);
    type resolve_call(call*);
    bool check_valid_left_value(expr*);
    void check_mutable_left_value(expr*, const type&);
    type resolve_assignment(assignment*);
    type resolve_expression(expr*);
    void resolve_definition(definition*, const colgm_func&);
    void check_defined_variable_is_void(definition*, const type&);
    void resolve_if_stmt(if_stmt*, const colgm_func&);
    bool check_is_match_default(expr*);
    bool check_is_enum_literal(expr*);
    size_t get_enum_literal_value(expr*, const type&);
    void resolve_cond_stmt(cond_stmt*, const colgm_func&);
    void resolve_match_stmt(match_stmt*, const colgm_func&);
    void resolve_while_stmt(while_stmt*, const colgm_func&);
    void resolve_for_stmt(for_stmt*, const colgm_func&);
    void lowering_forindex(forindex*);
    void resolve_forindex(forindex*, const colgm_func&);
    void lowering_foreach(foreach*);
    void resolve_foreach(foreach*, const colgm_func&);
    void resolve_in_stmt_expr(in_stmt_expr*, const colgm_func&);
    void resolve_ret_stmt(ret_stmt*, const colgm_func&);
    void resolve_statement(stmt*, const colgm_func&);
    void resolve_code_block(code_block*, const colgm_func&);
    void resolve_global_func(func_decl*);
    void resolve_method(func_decl*, const colgm_struct&);
    void resolve_method(func_decl*, const colgm_tagged_union&);
    void resolve_impl(impl*);
    void resolve_function_block(root*);

public:
    semantic(error& e):
        err(e), ctx(), rp(err), tr(err, ctx),
        in_loop_level(0), impl_struct_name("") {}
    const error& analyse(root*);
    void dump();
    const auto& get_context() const { return ctx; }
    void set_main_input_file(const std::string& input_file) {
        ctx.global.input_file = input_file;
    }
};

}