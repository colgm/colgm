#pragma once

#include "ast/visitor.h"
#include "sema/context.h"
#include "mir/mir.h"
#include "report.h"

#include <iostream>
#include <cstring>
#include <sstream>
#include <unordered_map>

namespace colgm::mir {

struct mir_func {
    std::string name;
    span location;
    std::vector<std::pair<std::string, type>> params;
    type return_type;
    mir_block* block;

    mir_func(): block(nullptr) {}
    ~mir_func() { delete block; }
};

struct mir_context {
    std::unordered_map<std::string, mir_func*> impls;
    ~mir_context() {
        for(auto i : impls) {
            delete i.second;
        }
    }
};

class ast2mir: public visitor {
private:
    error err;
    const semantic_context& ctx;

private:
    static inline mir_context mctx;
    std::string impl_struct_name = "";
    mir_block* block = nullptr;

private:
    bool visit_unary_operator(unary_operator*) override;
    bool visit_binary_operator(binary_operator*) override;
    bool visit_type_convert(type_convert*) override;
    bool visit_nil_literal(nil_literal*) override;
    bool visit_number_literal(number_literal*) override;
    bool visit_string_literal(string_literal*) override;
    bool visit_char_literal(char_literal*) override;
    bool visit_bool_literal(bool_literal*) override;
    bool visit_func_decl(func_decl*) override;
    bool visit_impl_struct(impl_struct*) override;
    bool visit_call_index(call_index*) override;
    bool visit_call_func_args(call_func_args*) override;
    bool visit_call_field(call_field*) override;
    bool visit_ptr_call_field(ptr_call_field*) override;
    bool visit_call_path(call_path*) override;
    bool visit_call(call*) override;
    bool visit_definition(definition*) override;
    bool visit_assignment(assignment*) override;
    bool visit_cond_stmt(cond_stmt*) override;
    mir_if* generate_if_stmt(if_stmt*);
    bool visit_while_stmt(while_stmt*) override;
    bool visit_continue_stmt(continue_stmt*);
    bool visit_break_stmt(break_stmt*);
    bool visit_code_block(code_block*) override;

private:
    type generate_type(type_def*);
    void emit_function(mir_func*);

public:
    ast2mir(const semantic_context& c): ctx(c) {}

public:
    static void dump(std::ostream&);
    auto& generate(root* ast_root) {
        ast_root->accept(this);
        return err;
    }
};

}