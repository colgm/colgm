#pragma once

#include "report.h"
#include "sema/context.h"
#include "ast/ast.h"

namespace colgm {

class regist_pass {
private:
    error& err;
    sema_context& ctx;

private:
    void report(ast::node* n, const std::string& info) {
        err.err("semantic", n->get_location(), info);
    }

private:
    bool check_is_public_struct(ast::identifier*, const colgm_module&);
    bool check_is_public_func(ast::identifier*, const colgm_module&);
    bool check_is_public_enum(ast::identifier*, const colgm_module&);
    void import_global_symbol(ast::node*, const std::string&, const symbol_info&);
    bool check_is_specified_enum_member(ast::number_literal*);

private:
    colgm_func builtin_struct_size(const span&);
    colgm_func builtin_struct_alloc(const span&, const type&);

private:
    void regist_basic_types();
    void regist_single_import(ast::use_stmt*);
    void regist_imported_types(ast::root*);
    void regist_enums(ast::root*);
    void regist_single_enum(ast::enum_decl*);
    void regist_structs(ast::root*);
    void regist_single_struct_symbol(ast::struct_decl*);
    void regist_single_struct_field(ast::struct_decl*);
    void check_struct_self_reference();
    void regist_funcs(ast::root*);
    void regist_single_func(ast::func_decl*);

public:
    regist_pass(error& e, sema_context& c): err(e), ctx(c) {}
    void run(ast::root*);
};

}