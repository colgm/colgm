#pragma once

#include "report.h"
#include "sema/context.h"
#include "sema/reporter.h"
#include "sema/type_resolver.h"
#include "ast/ast.h"
#include "ast/visitor.h"

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace colgm {

struct generic_data {
    std::string name;
    type generic_type;
    std::unordered_map<std::string, type> types;
};

class type_replace_pass: public ast::visitor {
private:
    error& err;
    sema_context& ctx;
    generic_data g_data;

private:
    ast::type_def* generate_generic_type(const type&, const span&);

private:
    bool visit_type_def(ast::type_def*) override;
    bool visit_call_id(ast::call_id*) override;

public:
    type_replace_pass(error& e, sema_context& c, const generic_data& gd):
        err(e), ctx(c), g_data(gd) {}
    void visit_func(ast::func_decl* node) {
        node->accept(this);
    }
    void visit_impl_node(ast::impl* node) {
        node->accept(this);
    }
    void visit_struct(ast::struct_decl* node) {
        node->accept(this);
    }
};

class generic_visitor: public ast::visitor {
private:
    const u64 MAX_RECURSIVE_DEPTH = 16;

private:
    error& err;
    sema_context& ctx;
    reporter rp;
    type_resolver tr;
    ast::root* root;

    u64 insert_count;
    std::vector<ast::decl*> need_to_be_inserted;

private:
    void scan_generic_type(ast::type_def*);
    void check_generic_type(ast::node*,
                            const std::string&,
                            const symbol_info&,
                            const std::vector<ast::type_def*>&,
                            const std::vector<std::string>&);

private:
    bool visit_struct_decl(ast::struct_decl*) override;
    bool visit_func_decl(ast::func_decl*) override;
    bool visit_impl(ast::impl*) override;
    bool visit_call_id(ast::call_id*) override;
    bool visit_type_def(ast::type_def*) override;

private:
    void replace_type(type&, const generic_data&);
    bool check_is_trivial(ast::cond_compile*, const generic_data&);
    bool check_is_non_trivial(ast::cond_compile*, const generic_data&);
    bool check_is_pointer(ast::cond_compile*, const generic_data&);
    bool check_is_non_pointer(ast::cond_compile*, const generic_data&);
    void remove_cond_compile_method(colgm_struct&, const generic_data&);
    void replace_struct_type(colgm_struct&, const generic_data&);
    void replace_func_type(colgm_func&, const generic_data&);

private:
    void visit(ast::root* n) {
        root = n;
        n->accept(this);
    }
    u64 insert_generic_data(const generic_data&);
    void report_recursive_generic_generation(const type&);

public:
    generic_visitor(error& e, sema_context& c):
        err(e), ctx(c), rp(e), tr(e, ctx), root(nullptr) {}
    void scan_and_insert(ast::root* n) {
        insert_count = 0;
        need_to_be_inserted.clear();
        visit(n);
        for (auto n: need_to_be_inserted) {
            root->add_decl(n);
        }
        while (insert_count) {
            insert_count = 0;
            need_to_be_inserted.clear();
            visit(n);
            for (auto n: need_to_be_inserted) {
                root->add_decl(n);
            }
        }
    }
};

class regist_pass {
private:
    error& err;
    sema_context& ctx;
    reporter rp;
    type_resolver tr;
    generic_visitor gnv;

private:
    bool check_is_public_struct(ast::identifier*, const colgm_module&);
    bool check_is_public_tagged_union(ast::identifier*, const colgm_module&);
    bool check_is_public_func(ast::identifier*, const colgm_module&);
    bool check_is_public_enum(ast::identifier*, const colgm_module&);
    void import_global_symbol(ast::node*,
                              const std::string&,
                              const symbol_info&,
                              bool);
    // specify enum member with number, but number should not be float,
    // so we need to check if this is float
    bool check_is_valid_enum_member(ast::number_literal*);

private:
    colgm_func builtin_size(const span&);
    colgm_func builtin_alloc(const span&, const type&);
    colgm_func builtin_ptr(const span&, const type&);

private: // primitive types
    // primitive types, now support:
    // bool, i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, void
    void regist_primitive_types();
    colgm_func generate_primitive_size_method();
    colgm_func generate_primitive_ptr_method(const char*);
    void regist_builtin_funcs();

private: // import symbols
    void regist_single_import(ast::use_stmt*, bool);
    void regist_imported_types(ast::root*, bool);

private: // enums
    void regist_enums(ast::root*);
    void regist_single_enum(ast::enum_decl*);

private: // structs and tagged unions
    void regist_complex_structs(ast::root*);
    void regist_single_struct_symbol(ast::struct_decl*);
    void regist_single_struct_field(ast::struct_decl*);
    void check_self_reference();
    void check_single_self_reference(const std::string&);
    void check_ref_enum(ast::tagged_union_decl*, colgm_tagged_union&);
    void load_tagged_union_member_map(ast::tagged_union_decl*, colgm_tagged_union&);
    void regist_single_tagged_union_symbol(ast::tagged_union_decl*);
    void regist_single_tagged_union_member(ast::tagged_union_decl*);

private: // global functions
    void regist_global_funcs(ast::root*);
    void regist_single_global_func(ast::func_decl*);
    colgm_func generate_single_global_func(ast::func_decl*);
    void generate_parameter_list(ast::param_list*, colgm_func&);
    void generate_parameter(ast::param*, colgm_func&);
    void generate_return_type(ast::type_def*, colgm_func&);

private: // implementations
    void regist_impls(ast::root*);
    void regist_single_impl(ast::impl*);
    void regist_single_impl_for_struct(ast::impl*, colgm_struct&);
    void regist_single_impl_for_tagged_union(ast::impl*, colgm_tagged_union&);
    colgm_func generate_method(ast::func_decl*, const colgm_struct&);
    colgm_func generate_method(ast::func_decl*, const colgm_tagged_union&);
    void generate_self_parameter(ast::param*, const colgm_struct&);
    void generate_self_parameter(ast::param*, const colgm_tagged_union&);
    void generate_method_parameter_list(ast::param_list*,
                                        colgm_func&,
                                        const colgm_struct&);
    void generate_method_parameter_list(ast::param_list*,
                                        colgm_func&,
                                        const colgm_tagged_union&);

public:
    regist_pass(error& e, sema_context& c):
        err(e), ctx(c), rp(e), tr(err, ctx), gnv(err, ctx) {}
    void run(ast::root*, bool);
};

}