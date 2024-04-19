#pragma once

#include "ast/visitor.h"
#include "code/sir.h"
#include "code/hir.h"
#include "code/value.h"
#include "code/context.h"
#include "semantic.h"
#include "report.h"

#include <vector>
#include <cstring>
#include <sstream>

namespace colgm {

class generator: public visitor {
private:
    i64 ssa_temp_counter = 0;
    std::string get_temp_variable() {
        return std::to_string(ssa_temp_counter++);
    }
    usize place_holder_label = 0;
    auto get_place_holder_label() {
        return place_holder_label++;
    }

private:
    const semantic_context& ctx;
    error err;
    void unreachable(node* n) {
        err.err("code", n->get_location(), "unreachable.");
    }
    void unimplemented(node* n) {
        err.err("code", n->get_location(),
            "unimplemented. please report an issue."
        );
    }
    void report_stack_empty(node* n) {
        err.err("code", n->get_location(), "internal error: stack is empty.");
    }

private:
    const std::unordered_map<std::string, std::string> basic_type_convert_mapper = {
        {"i64", "i64"},
        {"i32", "i32"},
        {"i16", "i16"},
        {"i8", "i8"},
        {"u64", "i64"},
        {"u32", "i32"},
        {"u16", "i16"},
        {"u8", "i8"},
        {"f64", "double"},
        {"f32", "float"},
        {"void", "void"},
        {"bool", "i1"}
    };
    std::string type_convert(const type&);

private:
    static inline ir_context irc;

    std::string impl_struct_name = "";
    sir_block* ircode_block = nullptr;

    void emit_func_impls(hir_func* i) { irc.func_impls.push_back(i); }
    void emit_func_decl(hir_func* f) { irc.func_decls.push_back(f); }
    void emit_struct_decl(const hir_struct& s) { irc.struct_decls.push_back(s); }
    std::string generate_type_string(type_def*);
    bool visit_struct_decl(struct_decl*) override;
    void convert_parameter_to_pointer(func_decl*);
    bool visit_func_decl(func_decl*) override;
    bool visit_impl_struct(impl_struct*) override;
    bool visit_code_block(code_block*) override;

private:
    std::vector<value> value_stack;

    void call_expression_generation(call*, bool);

    bool visit_number_literal(number_literal*) override;
    bool visit_string_literal(string_literal*) override;
    bool visit_char_literal(char_literal*) override;
    bool visit_bool_literal(bool_literal*) override;
    void call_function_symbol(identifier*);
    void call_variable(identifier*);
    bool visit_call(call*) override;
    bool visit_call_index(call_index*) override;
    bool visit_call_field(call_field*) override;
    bool visit_ptr_call_field(ptr_call_field*) override;
    bool visit_call_path(call_path*) override;
    bool visit_call_func_args(call_func_args*) override;
    void generate_and_operator(binary_operator*);
    void generate_or_operator(binary_operator*);
    void generate_add_operator(const value&, const value&, const value&);
    void generate_sub_operator(const value&, const value&, const value&);
    void generate_mul_operator(const value&, const value&, const value&);
    void generate_div_operator(const value&, const value&, const value&);
    void generate_rem_operator(const value&, const value&, const value&);
    void generate_band_operator(const value&, const value&, const value&);
    void generate_bxor_operator(const value&, const value&, const value&);
    void generate_bor_operator(const value&, const value&, const value&);
    void generate_eq_operator(const value&, const value&, const value&);
    void generate_neq_operator(const value&, const value&, const value&);
    void generate_ge_operator(const value&, const value&, const value&);
    void generate_gt_operator(const value&, const value&, const value&);
    void generate_le_operator(const value&, const value&, const value&);
    void generate_lt_operator(const value&, const value&, const value&);
    bool visit_binary_operator(binary_operator*) override;
    bool visit_unary_operator(unary_operator*) override;
    bool visit_ret_stmt(ret_stmt*) override;
    bool visit_definition(definition*) override;
    void generate_add_assignment(const value&, const value&);
    void generate_sub_assignment(const value&, const value&);
    void generate_mul_assignment(const value&, const value&);
    void generate_div_assignment(const value&, const value&);
    void generate_rem_assignment(const value&, const value&);
    void generate_eq_assignment(const value&, const value&);
    void generate_and_assignment(const value&, const value&);
    void generate_xor_assignment(const value&, const value&);
    void generate_or_assignment(const value&, const value&);
    bool visit_assignment(assignment*) override;

private:
    std::vector<usize> loop_begin;
    std::vector<sir_br*> break_inst;

    bool visit_while_stmt(while_stmt*) override;
    bool visit_continue_stmt(continue_stmt*) override;
    bool visit_break_stmt(break_stmt*) override;

private:
    std::vector<sir_br*> jump_outs;

    bool visit_if_stmt(if_stmt*) override;
    bool visit_cond_stmt(cond_stmt*) override;

public:
    generator(const semantic_context& c): ctx(c) {}
    auto& generate(root* ast_root) {
        ast_root->accept(this);
        return err;
    }
    const auto& get_ir() const { return irc; }
};

}