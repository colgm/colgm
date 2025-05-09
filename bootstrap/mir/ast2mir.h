#pragma once

#include "colgm.h"
#include "ast/visitor.h"
#include "sema/context.h"
#include "sema/type_resolver.h"
#include "mir/mir.h"
#include "report.h"

#include <iostream>
#include <cstring>
#include <sstream>
#include <unordered_map>

namespace colgm::mir {

struct mir_struct {
    std::string name;
    span location;
    std::vector<type> field_type;
};

struct mir_func {
    std::string name;
    span location;
    std::vector<std::pair<std::string, type>> params;
    bool with_va_args;
    std::vector<std::string> attributes;
    type return_type;
    mir_block* block;

    mir_func(): with_va_args(false), block(nullptr) {}
    ~mir_func() { delete block; }
};

struct mir_context {
    std::vector<mir_struct*> structs;
    std::vector<mir_func*> decls;
    std::vector<mir_func*> impls;

    ~mir_context() {
        for(auto i : structs) {
            delete i;
        }
        for(auto i : decls) {
            delete i;
        }
        for(auto i : impls) {
            delete i;
        }
    }
};

class ast2mir: public ast::visitor {
private:
    error& err;
    const sema_context& ctx;
    type_resolver tr;

private:
    static inline mir_context mctx;
    std::string impl_struct_name = "";
    mir_block* block = nullptr;

private:
    bool visit_enum_decl(ast::enum_decl*) override { return true; }
    bool visit_unary_operator(ast::unary_operator*) override;
    bool visit_binary_operator(ast::binary_operator*) override;
    bool visit_type_convert(ast::type_convert*) override;
    bool visit_nil_literal(ast::nil_literal*) override;
    bool visit_number_literal(ast::number_literal*) override;
    bool visit_string_literal(ast::string_literal*) override;
    bool visit_char_literal(ast::char_literal*) override;
    bool visit_bool_literal(ast::bool_literal*) override;
    bool visit_array_list(ast::array_list*) override;
    bool visit_struct_decl(ast::struct_decl*) override;
    bool visit_func_decl(ast::func_decl*) override;
    bool visit_impl_struct(ast::impl_struct*) override;
    bool visit_call_index(ast::call_index*) override;
    bool visit_call_func_args(ast::call_func_args*) override;
    bool visit_get_field(ast::get_field*) override;
    bool visit_ptr_get_field(ast::ptr_get_field*) override;
    bool visit_initializer(ast::initializer*) override;
    bool visit_call_path(ast::call_path*) override;
    bool visit_call(ast::call*) override;
    bool visit_definition(ast::definition*) override;
    bool visit_assignment(ast::assignment*) override;
    bool visit_cond_stmt(ast::cond_stmt*) override;
    mir_if* generate_if_stmt(ast::if_stmt*);
    bool visit_match_stmt(ast::match_stmt*) override;
    mir_switch_case* generate_match_case(ast::match_case*);
    mir_block* generate_default(ast::match_case*);
    bool visit_while_stmt(ast::while_stmt*) override;
    bool visit_for_stmt(ast::for_stmt*) override;
    bool visit_forindex(ast::forindex*) override;
    bool visit_foreach(ast::foreach*) override;
    bool visit_ret_stmt(ast::ret_stmt*) override;
    bool visit_continue_stmt(ast::continue_stmt*) override;
    bool visit_break_stmt(ast::break_stmt*) override;
    bool visit_code_block(ast::code_block*) override;

private:
    type generate_type(ast::type_def*);

public:
    ast2mir(error& e, const sema_context& c): err(e), ctx(c), tr(e, c) {}

public:
    static void dump(std::ostream&);
    auto& generate(ast::root* ast_root) {
        ast_root->accept(this);
        return err;
    }
    static auto get_context() { return &mctx; }
};

}