#pragma once

#include "ast/visitor.h"
#include "code/ir.h"
#include "code/hir.h"
#include "semantic.h"

#include <vector>
#include <cstring>
#include <sstream>

namespace colgm {

struct ir_context {
    std::vector<hir_struct> struct_decls;
    std::vector<hir_func*> func_decls;
    std::vector<hir_func*> func_impls;

    ~ir_context() {
        for(auto i : func_decls) {
            delete i;
        }
        for(auto i : func_impls) {
            delete i;
        }
    }
    void dump_code(std::ostream&) const;
};

class ir_gen: public visitor {
private:
    const semantic_context& ctx;
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

private:
    static inline ir_context irs;

    std::string impl_struct_name = "";
    ir_code_block* cb = nullptr;

    void emit_func_impls(hir_func* i) { irs.func_impls.push_back(i); }
    void emit_func_decl(hir_func* f) { irs.func_decls.push_back(f); }
    void emit_struct_decl(const hir_struct& s) { irs.struct_decls.push_back(s); }
    std::string generate_type_string(type_def*);
    bool visit_struct_decl(struct_decl*) override;
    bool visit_func_decl(func_decl*) override;
    bool visit_impl_struct(impl_struct*) override;
    bool visit_number_literal(number_literal*) override;
    bool visit_string_literal(string_literal*) override;
    bool visit_bool_literal(bool_literal*) override;
    bool visit_call(call*) override;
    bool visit_call_index(call_index*) override;
    bool visit_call_field(call_field*) override;
    bool visit_ptr_call_field(ptr_call_field*) override;
    bool visit_call_path(call_path*) override;
    bool visit_call_func_args(call_func_args*) override;
    bool visit_binary_operator(binary_operator*) override;
    bool visit_ret_stmt(ret_stmt*) override;
    bool visit_definition(definition*) override;
    bool visit_assignment(assignment*) override;
    bool visit_while_stmt(while_stmt*) override;
    bool visit_if_stmt(if_stmt*) override;

public:
    ir_gen(const semantic_context& c): ctx(c) {}
    void generate(root* ast_root) { ast_root->accept(this); }
    const auto& get_ir() const { return irs; }
};

}