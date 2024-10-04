#pragma once

#include "report.h"
#include "sema/context.h"
#include "sema/reporter.h"
#include "sema/resolver.h"
#include "ast/ast.h"
#include "ast/visitor.h"

namespace colgm {

class generic_visitor: public ast::visitor {
private:
    sema_context& ctx;
    reporter rp;

private:
    bool visit_call_id(ast::call_id*) override;

public:
    generic_visitor(error& e, sema_context& c): ctx(c), rp(e) {}
    void visit(ast::root* n) { n->accept(this); }
};

class regist_pass {
private:
    error& err;
    sema_context& ctx;
    reporter rp;
    resolver rs;
    generic_visitor gnv;

private:
    bool check_is_public_struct(ast::identifier*, const colgm_module&);
    bool check_is_public_func(ast::identifier*, const colgm_module&);
    bool check_is_public_enum(ast::identifier*, const colgm_module&);
    void import_global_symbol(ast::node*,
                              const std::string&,
                              const symbol_info&);
    bool check_is_specified_enum_member(ast::number_literal*);

private:
    colgm_func builtin_struct_size(const span&);
    colgm_func builtin_struct_alloc(const span&, const type&);

private: // basic types
    // basic types, now support:
    // bool, i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, void
    void regist_basic_types();

private: // import symbols
    void regist_single_import(ast::use_stmt*);
    void regist_imported_types(ast::root*);

private: // enums
    void regist_enums(ast::root*);
    void regist_single_enum(ast::enum_decl*);

private: // structs
    void regist_structs(ast::root*);
    void regist_single_struct_symbol(ast::struct_decl*);
    void regist_single_struct_field(ast::struct_decl*);
    void check_struct_self_reference();

private: // global functions
    void regist_global_funcs(ast::root*);
    void regist_single_global_func(ast::func_decl*);
    colgm_func generate_single_global_func(ast::func_decl*);
    void generate_parameter_list(ast::param_list*, colgm_func&);
    void generate_parameter(ast::param*, colgm_func&);
    void generate_return_type(ast::type_def*, colgm_func&);

private: // implementations
    void regist_impls(ast::root*);
    void regist_single_impl(ast::impl_struct*);
    colgm_func generate_method(ast::func_decl*, const colgm_struct&);
    void generate_method_parameter_list(ast::param_list*,
                                        colgm_func&,
                                        const colgm_struct&);

public:
    regist_pass(error& e, sema_context& c):
        err(e), ctx(c), rp(e), rs(err, ctx), gnv(err, ctx) {}
    void run(ast::root*);
};

}