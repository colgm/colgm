#include "ast/stmt.h"
#include "ast/expr.h"
#include "sema/regist_pass.h"
#include "lexer.h"
#include "parse/parse.h"
#include "sema/semantic.h"
#include "mir/ast2mir.h"

#include <cassert>
#include <cstring>
#include <sstream>
#include <queue>
#include <unordered_map>

namespace colgm {

ast::type_def* type_replace_pass::generate_generic_type(const type& t,
                                                        const span& loc) {
    auto new_def = new type_def(loc);
    new_def->set_name(new identifier(loc, t.name));
    new_def->get_name()->set_redirect_location(t.loc_file);
    new_def->set_redirect_location(t.loc_file);
    if (t.is_const) {
        new_def->set_constant();
    }
    for (i64 i = 0; i < t.pointer_depth; ++i) {
        new_def->add_pointer_level();
    }
    if (!t.generics.empty()) {
        new_def->set_generic_types(new generic_type_list(loc));
        for (const auto& i : t.generics) {
            new_def->get_generic_types()->add_type(
                generate_generic_type(i, loc)
            );
        }
    }
    return new_def;
}

bool type_replace_pass::visit_type_def(type_def* node) {
    const auto name = node->get_name()->get_name();
    if (g_data.types.count(name)) {
        const auto& select_type = g_data.types.at(name);
        node->get_name()->reset_name(select_type.name);
        node->get_name()->set_redirect_location(select_type.loc_file);
        node->set_redirect_location(select_type.loc_file);

        for (i64 i = 0; i < select_type.pointer_depth; ++i) {
            node->add_pointer_level();
        }

        if (!select_type.generics.empty() &&
            node->get_generic_types()) {
            err.err(node->get_name()->get_location(),
                "replace type \"" + select_type.full_path_name() +
                "\" is already a generic type, not allowed to replace \"" +
                name + "\"."
            );
        }
        if (!select_type.generics.empty() &&
            !node->get_generic_types()) {
            node->set_generic_types(new generic_type_list(node->get_location()));
            for (const auto& i : select_type.generics) {
                node->get_generic_types()->add_type(
                    generate_generic_type(i, node->get_location())
                );
            }
        }
    }
    if (node->get_generic_types()) {
        node->get_generic_types()->accept(this);
    }
    return true;
}

bool type_replace_pass::visit_call_id(ast::call_id* node) {
    const auto name = node->get_id()->get_name();
    if (g_data.types.count(name)) {
        const auto& select_type = g_data.types.at(name);
        node->get_id()->set_name(select_type.name);
        node->get_id()->set_redirect_location(select_type.loc_file);
        node->set_redirect_location(select_type.loc_file);

        if (select_type.pointer_depth) {
            err.err(node->get_id()->get_location(),
                "replace type \"" + select_type.full_path_name() +
                "\" is a pointer type, which is not allowed here."
            );
        }

        if (!select_type.generics.empty() &&
            node->get_generic_types()) {
            err.err(node->get_id()->get_location(),
                "replace type \"" + select_type.full_path_name() +
                "\" is already a generic type, not allowed to replace \"" +
                name + "\"."
            );
        }
        if (!select_type.generics.empty() &&
            !node->get_generic_types()) {
            node->set_generic_types(new generic_type_list(node->get_location()));
            for (const auto& i : select_type.generics) {
                node->get_generic_types()->add_type(
                    generate_generic_type(i, node->get_location())
                );
            }
        }
    }
    if (node->get_generic_types()) {
        node->get_generic_types()->accept(this);
    }
    return true;
}

void generic_visitor::scan_generic_type(type_def* type_node) {
    const auto& type_name = type_node->get_name()->get_name();
    const auto& dm = type_node->is_redirected()
        ? ctx.get_domain(type_node->get_redirect_location())
        : ctx.get_domain(type_node->get_file());

    if (!dm.global_symbol.count(type_name)) {
        rp.report(type_node, "unknown type \"" + type_name + "\".");
        return;
    }
    if (!dm.generic_symbol.count(type_name)) {
        rp.report(type_node->get_name(),
            "\"" + type_name + "\" is not a generic type."
        );
        return;
    }

    const auto& sym = dm.global_symbol.at(type_name);
    const auto& sdm = ctx.get_domain(sym.loc_file);
    if (sym.kind != sym_kind::struct_kind) {
        rp.report(type_node, "\"" + type_name + "\" is not a struct type.");
        return;
    }

    const auto& generic_template = sdm.generic_structs.at(type_name).generic_template;
    const auto& type_list = type_node->get_generic_types()->get_types();
    check_generic_type(type_node, type_name, sym, type_list, generic_template);
}

void generic_visitor::check_generic_type(
    node* node,
    const std::string& type_name,
    const symbol_info& sym,
    const std::vector<type_def*>& type_list,
    const std::vector<std::string>& generic_template) {

    // check size match
    if (type_list.size() != generic_template.size()) {
        rp.report(node, "generic type count does not match.");
        return;
    }

    // generate real name
    std::stringstream ss;
    ss << type_name << "<";
    for (auto i : type_list) {
        const auto type = tr.resolve(i);
        ss << type.full_path_name_with_pointer();
        if (i != type_list.back()) {
            ss << ",";
        }
    }
    ss << ">";
    if (rp.has_error()) {
        return;
    }

    // insert data, use symbols location file to avoid duplication of name

    auto rec = generic_data {};
    rec.name = ss.str();
    rec.generic_type.name = type_name;
    rec.generic_type.loc_file = sym.loc_file;

    for (i64 i = 0; i < generic_template.size(); ++i) {
        auto t = type_list[i];
        const auto resolved_type = tr.resolve(t);
        rec.generic_type.generics.push_back(resolved_type);
        rec.types.insert({generic_template[i], resolved_type});
    }

    insert_count += insert_generic_data(rec);
}

bool generic_visitor::visit_struct_decl(struct_decl* node) {
    // do not scan generic struct block
    if (node->get_generic_types()) {
        return true;
    }
    for (auto i : node->get_fields()) {
        if (!i->get_type() || !i->get_type()->get_generic_types()) {
            continue;
        }
        i->get_type()->accept(this);
    }
    return true;
}

bool generic_visitor::visit_func_decl(func_decl* node) {
    // do not scan generic function block
    if (node->get_generic_types()) {
        return true;
    }
    for (auto i : node->get_params()->get_params()) {
        i->accept(this);
        if (!i->get_type() || !i->get_type()->get_generic_types()) {
            continue;
        }
        i->get_type()->accept(this);
    }
    if (node->get_return_type()) {
        node->get_return_type()->accept(this);
    }
    if (node->get_code_block()) {
        node->get_code_block()->accept(this);
    }
    return true;
}

bool generic_visitor::visit_impl_struct(ast::impl_struct* node) {
    // do not scan generic impl block
    if (node->get_generic_types()) {
        return true;
    }
    for (auto i : node->get_methods()) {
        i->accept(this);
    }
    return true;
}

bool generic_visitor::visit_call_id(ast::call_id* node) {
    if (!node->get_generic_types()) {
        return true;
    }
    node->get_generic_types()->accept(this);

    const auto& type_name = node->get_id()->get_name();
    const auto& dm = node->is_redirected()
        ? ctx.get_domain(node->get_redirect_location())
        : ctx.get_domain(node->get_file());
    if (!dm.global_symbol.count(type_name)) {
        rp.report(node, "unknown type \"" + type_name + "\".");
        return true;
    }
    if (!dm.generic_symbol.count(type_name)) {
        rp.report(node->get_id(),
            "\"" + type_name + "\" is not a generic type."
        );
        return true;
    }

    const auto& sym = dm.global_symbol.at(type_name);
    const auto& sdm = ctx.get_domain(sym.loc_file);
    const auto& generic_template = sym.kind == sym_kind::struct_kind
        ? sdm.generic_structs.at(type_name).generic_template
        : sdm.generic_functions.at(type_name).generic_template;
    const auto& type_list = node->get_generic_types()->get_types();
    check_generic_type(node, type_name, sym, type_list, generic_template);
    return true;
}

bool generic_visitor::visit_type_def(ast::type_def* node) {
    if (node->get_generic_types()) {
        // dfs try to initialize generic input types first
        // then try to initialize the outside type
        node->get_generic_types()->accept(this);
        scan_generic_type(node);
    }
    return true;
}

void generic_visitor::replace_type(type& t, const generic_data& data) {
    if (data.types.count(t.name)) {
        const auto& real = data.types.at(t.name);
        t.name = real.name;
        t.loc_file = real.loc_file;
        t.generics = real.generics;
        // should add to t.pointer_depth, for example: func foo(a: T*) {}
        // we should replace T with bar, expected result is: bar*
        // if we do not add pointer_depth, the result will be bar
        t.pointer_depth += real.pointer_depth;
    }
    if (t.generics.empty()) {
        return;
    }
    for (auto& g : t.generics) {
        replace_type(g, data);
    }
}

bool generic_visitor::check_is_trivial(ast::cond_compile* node,
                                       const generic_data& data) {
    for (const auto& i : node->get_ordered_cond_name()) {
        if (!node->get_conds().at(i).empty()) {
            err.warn(node->get_location(), "condition value is ignored.");
        }
        if (!data.types.count(i)) {
            err.err(node->get_location(), "generic type '" + i + "' is not defined.");
            return false;
        }
        const auto& t = data.types.at(i);
        // pointer type is trivial
        if (t.is_pointer()) {
            continue;
        }
        // primitive type is trivial
        if (t.loc_file.empty()) {
            continue;
        }
        const auto& domain = ctx.get_domain(t.loc_file);
        // not a struct, must be trivial
        if (!domain.structs.count(t.generic_name())) {
            continue;
        }
        const auto& s = domain.structs.at(t.generic_name());
        // with delete method, it's non-trivial
        if (s.method.count("delete")) {
            return false;
        }
    }
    return true;
}

bool generic_visitor::check_is_non_trivial(ast::cond_compile* node,
                                           const generic_data& data) {
    for (const auto& i : node->get_ordered_cond_name()) {
        if (!node->get_conds().at(i).empty()) {
            err.warn(node->get_location(), "condition value is ignored.");
        }
        if (!data.types.count(i)) {
            err.err(node->get_location(), "generic type '" + i + "' is not defined.");
            return false;
        }
        const auto& t = data.types.at(i);
        if (t.is_pointer()) {
            return false;
        }
        if (t.loc_file.empty()) {
            // primitive type, is trivial
            return false;
        }
        const auto& domain = ctx.get_domain(t.loc_file);
        if (!domain.structs.count(t.generic_name())) {
            return false;
        }
        const auto& s = domain.structs.at(t.generic_name());
        // without delete method, it's trivial
        if (!s.method.count("delete")) {
            return false;
        }
    }
    return true;
}

bool generic_visitor::check_is_pointer(ast::cond_compile* node,
                                       const generic_data& data) {
    for (const auto& i : node->get_ordered_cond_name()) {
        if (!node->get_conds().at(i).empty()) {
            err.warn(node->get_location(), "condition value is ignored.");
        }
        if (!data.types.count(i)) {
            err.err(node->get_location(), "generic type '" + i + "' is not defined.");
            return false;
        }
        const auto& t = data.types.at(i);
        if (!t.is_pointer()) {
            return false;
        }
    }
    return true;
}

bool generic_visitor::check_is_non_pointer(ast::cond_compile* node,
                                           const generic_data& data) {
    for (const auto& i : node->get_ordered_cond_name()) {
        if (!node->get_conds().at(i).empty()) {
            err.warn(node->get_location(), "condition value is ignored.");
        }
        if (!data.types.count(i)) {
            err.err(node->get_location(), "generic type '" + i + "' is not defined.");
            return false;
        }
        const auto& t = data.types.at(i);
        if (t.is_pointer()) {
            return false;
        }
    }
    return true;
}

void generic_visitor::remove_cond_compile_method(colgm_struct& s,
                                                 const generic_data& data) {
    for (auto i : s.generic_struct_impl) {
        std::vector<ast::func_decl*> new_vec;
        for (auto j : i->get_methods()) {
            if (!j->contain_cond()) {
                new_vec.push_back(j);
                continue;
            }

            auto monomorphic_name = j->get_monomorphic_name();

            auto trivial_cond = j->get_trivial_cond();
            if (trivial_cond && !check_is_trivial(trivial_cond, data)) {
                delete j;
                if (s.method.count(monomorphic_name)) {
                    s.method.erase(monomorphic_name);
                } else if (s.static_method.count(monomorphic_name)) {
                    s.static_method.erase(monomorphic_name);
                }
                continue;
            }

            auto non_trivial_cond = j->get_non_trivial_cond();
            if (non_trivial_cond && !check_is_non_trivial(non_trivial_cond, data)) {
                delete j;
                if (s.method.count(monomorphic_name)) {
                    s.method.erase(monomorphic_name);
                } else if (s.static_method.count(monomorphic_name)) {
                    s.static_method.erase(monomorphic_name);
                }
                continue;
            }

            auto is_pointer_cond = j->get_is_pointer_cond();
            if (is_pointer_cond && !check_is_pointer(is_pointer_cond, data)) {
                delete j;
                if (s.method.count(monomorphic_name)) {
                    s.method.erase(monomorphic_name);
                } else if (s.static_method.count(monomorphic_name)) {
                    s.static_method.erase(monomorphic_name);
                }
                continue;
            }

            auto is_non_pointer_cond = j->get_is_non_pointer_cond();
            if (is_non_pointer_cond && !check_is_non_pointer(is_non_pointer_cond, data)) {
                delete j;
                if (s.method.count(monomorphic_name)) {
                    s.method.erase(monomorphic_name);
                } else if (s.static_method.count(monomorphic_name)) {
                    s.static_method.erase(monomorphic_name);
                }
                continue;
            }

            if (s.method.count(monomorphic_name)) {
                if (s.field.count(j->get_name()) ||
                    s.method.count(j->get_name()) ||
                    s.static_method.count(j->get_name())) {
                    err.err(j->get_location(), "method name conflicts.");
                } else {
                    s.method[j->get_name()] = s.method.at(monomorphic_name);
                    s.method.at(j->get_name()).name = j->get_name();
                }
                s.method.erase(monomorphic_name);
            } else if (s.static_method.count(monomorphic_name)) {
                if (s.field.count(j->get_name()) ||
                    s.method.count(j->get_name()) ||
                    s.static_method.count(j->get_name())) {
                    err.err(j->get_location(), "static method name conflicts.");
                } else {
                    s.static_method[j->get_name()] = s.static_method.at(monomorphic_name);
                    s.static_method.at(j->get_name()).name = j->get_name();
                }
                s.static_method.erase(monomorphic_name);
            }
            new_vec.push_back(j);
        }
        if (new_vec.size() != i->get_methods().size()) {
            i->reset_methods(new_vec);
        }
    }
}

void generic_visitor::replace_struct_type(colgm_struct& s,
                                          const generic_data& data) {
    for (auto& i : s.field) {
        replace_type(i.second, data);
    }

    // remove method by conditonal compile config like
    // #[is_trivial(T, K)]
    // #[is_non_trivial(T, K)]
    remove_cond_compile_method(s, data);

    for (auto& i : s.method) {
        replace_func_type(i.second, data);
    }
    for (auto& i : s.static_method) {
        replace_func_type(i.second, data);
    }

    type_replace_pass trp(err, ctx, data);
    if (s.generic_struct_decl) {
        trp.visit_struct(s.generic_struct_decl);
        need_to_be_inserted.push_back(s.generic_struct_decl);
        // s.name is generated with generic
        // for example original name is "foo",
        // but now it should be replaced with "foo<int, bool>"
        s.generic_struct_decl->set_name(s.name);
        s.generic_struct_decl->clear_generic_types();
        s.generic_struct_decl = nullptr;
    }
    for (auto i : s.generic_struct_impl) {
        trp.visit_impl(i);
        need_to_be_inserted.push_back(i);
        // s.name is generated with generic
        // for example original name is "foo",
        // but now it should be replaced with "foo<int, bool>"
        i->set_struct_name(s.name);
        i->clear_generic_types();
    }
    s.generic_struct_impl.clear();
    return;
}

void generic_visitor::replace_func_type(colgm_func& f,
                                        const generic_data& data) {
    // replace return type
    replace_type(f.return_type, data);

    // replace parameter type
    for (auto& i : f.params) {
        replace_type(i.second, data);
    }

    if (f.generic_func_decl) {
        type_replace_pass trp(err, ctx, data);
        trp.visit_func(f.generic_func_decl);
        need_to_be_inserted.push_back(f.generic_func_decl);
        // f.name is generated with generic
        // for example original name is "foo",
        // but now it should be replaced with "foo<int, bool>"
        f.generic_func_decl->set_name(f.name);
        f.generic_func_decl->clear_generic_types();
    }
    f.generic_func_decl = nullptr;
}

void generic_visitor::report_recursive_generic_generation(const type& data) {
    if (!ctx.global.domain.count(data.loc_file)) {
        return;
    }
    const auto& dm = ctx.get_domain(data.loc_file);
    const auto generic_name = data.generic_name();
    if (dm.structs.count(generic_name) || dm.functions.count(generic_name)) {
        return;
    }

    const auto& loc = dm.generic_structs.count(data.name)
        ? dm.generic_structs.at(data.name).location
        : dm.generic_functions.at(data.name).location;

    auto info = std::string("template instantiation depth exceeds maximum of ");
    info += std::to_string(MAX_RECURSIVE_DEPTH) + ":\n";
    info += "  --> " + generic_name;
    err.err(loc, info);
}

u64 generic_visitor::insert_generic_data(const generic_data& gd) {
    if (gd.generic_type.generic_depth() > MAX_RECURSIVE_DEPTH) {
        report_recursive_generic_generation(gd.generic_type);
        return 0;
    }

    const auto& data = gd.generic_type;
    const auto generic_name = data.generic_name();

    if (!ctx.global.domain.count(data.loc_file)) {
        return 0;
    }

    auto& dm = ctx.get_domain(data.loc_file);
    if (dm.generic_structs.count(data.name)) {
        // no need to load again, otherwise will cause redefine error
        if (dm.structs.count(generic_name)) {
            return 0;
        }

        dm.structs.insert({
            generic_name,
            dm.generic_structs.at(data.name)
        });
        dm.structs.at(generic_name).name = generic_name;
        dm.structs.at(generic_name).generic_template.clear();
        ctx.insert(generic_name, symbol_info {
            .kind = sym_kind::struct_kind,
            .loc_file = data.loc_file,
            .is_public = dm.structs.at(generic_name).is_public
        });
        replace_struct_type(dm.structs.at(generic_name), gd);
    } else if (dm.generic_functions.count(data.name)) {
        // no need to load again, otherwise will cause redefine error
        if (dm.functions.count(generic_name)) {
            return 0;
        }

        dm.functions.insert({
            generic_name,
            dm.generic_functions.at(data.name)
        });
        dm.functions.at(generic_name).name = generic_name;
        dm.functions.at(generic_name).generic_template.clear();
        ctx.insert(generic_name, symbol_info {
            .kind = sym_kind::func_kind,
            .loc_file = data.loc_file,
            .is_public = dm.functions.at(generic_name).is_public
        });
        replace_func_type(dm.functions.at(generic_name), gd);
    }

    return 1;
}

bool regist_pass::check_is_public_struct(ast::identifier* node,
                                         const colgm_module& domain) {
    const auto& name = node->get_name();
    if (!domain.structs.count(name) &&
        !domain.generic_structs.count(name)) {
        return false;
    }
    if (domain.structs.count(name) &&
        !domain.structs.at(name).is_public) {
        rp.report(node,
            "cannot import private struct \"" + name + "\"."
        );
        return false;
    }
    if (domain.generic_structs.count(name) &&
        !domain.generic_structs.at(name).is_public) {
        rp.report(node,
            "cannot import private struct \"" + name + "\"."
        );
        return false;
    }
    return true;
}

bool regist_pass::check_is_public_func(ast::identifier* node,
                                       const colgm_module& domain) {
    const auto& name = node->get_name();
    if (!domain.functions.count(name) &&
        !domain.generic_functions.count(name)) {
        return false;
    }
    if (domain.functions.count(name) &&
        !domain.functions.at(name).is_public) {
        rp.report(node,
            "cannot import private function \"" + name + "\"."
        );
        return false;
    }
    if (domain.generic_functions.count(name) &&
        !domain.generic_functions.at(name).is_public) {
        rp.report(node,
            "cannot import private function \"" + name + "\"."
        );
        return false;
    }
    return true;
}

bool regist_pass::check_is_public_enum(ast::identifier* node,
                                       const colgm_module& domain) {
    if (!domain.enums.count(node->get_name())) {
        return false;
    }
    if (!domain.enums.at(node->get_name()).is_public) {
        rp.report(node,
            "cannot import private enum \"" +
            node->get_name() + "\"."
        );
        return false;
    }
    return true;
}

void regist_pass::import_global_symbol(ast::node* n, 
                                       const std::string& name,
                                       const symbol_info& sym,
                                       bool is_generic) {
    if (ctx.global_symbol().count(name)) {
        const auto& info = ctx.global_symbol().at(name);
        rp.report(n, "\"" + name +
            "\" conflicts, another declaration is in \"" +
            info.loc_file + "\"."
        );
        return;
    }

    ctx.insert(name, sym, is_generic);
}

bool regist_pass::check_is_valid_enum_member(ast::number_literal* node) {
    const auto& num_str = node->get_number();
    if (num_str.find('.') != std::string::npos ||
        num_str.find('e') != std::string::npos) {
        return false;
    }
    f64 num = str_to_num(num_str.c_str());
    if (std::isinf(num) || std::isnan(num)) {
        return false;
    }
    if (num - std::floor(num) != 0) {
        return false;
    }
    return true;
}

colgm_func regist_pass::builtin_struct_size(const span& loc) {
    auto func = colgm_func();
    func.name = "__size__";
    func.location = loc;
    func.return_type = type::u64_type();
    func.is_public = true;
    return func;
}

colgm_func regist_pass::builtin_struct_alloc(const span& loc, const type& ty) {
    auto func = colgm_func();
    func.name = "__alloc__";
    func.location = loc;
    func.return_type = ty;
    func.is_public = true;
    return func;
}

colgm_func regist_pass::builtin_struct_ptr(const span& loc, const type& ty) {
    auto func = colgm_func();
    func.name = "__ptr__";
    func.location = loc;
    func.add_parameter("self", ty);
    func.return_type = ty;
    func.is_public = true;
    return func;
}

void regist_pass::regist_primitive_types() {
    const char* primitives[] = {
        "i64", "i32", "i16", "i8",
        "u64", "u32", "u16", "u8",
        "f32", "f64", "void", "bool"
    };

    // do clear, so this process should be called first
    ctx.global_symbol().clear();

    // load symbol info
    for (auto i : primitives) {
        ctx.global_symbol().insert({i, {sym_kind::basic_kind, "", true}});
    }

    // load default methods
    for (auto i : primitives) {
        if (std::string(i) == "void") {
            continue;
        }
        // avoid duplicate
        if (ctx.global.primitives.count(i)) {
            continue;
        }
        ctx.global.primitives.insert({i, colgm_primitive()});
        auto& cp = ctx.global.primitives.at(i);
        cp.name = i;
        cp.static_methods.insert({
            "__size__",
            generate_primitive_size_method()
        });
    }
}

colgm_func regist_pass::generate_primitive_size_method() {
    auto func = colgm_func();
    func.name = "__size__";
    func.location = span::null();
    func.return_type = type::u64_type();
    func.is_public = true;
    return func;
}

void regist_pass::regist_builtin_funcs() {
    // insert into symbol table
    ctx.insert(
        "__time__",
        symbol_info {sym_kind::func_kind, ctx.this_file, true},
        false
    );

    auto time_func = colgm_func();
    time_func.name = "__time__";
    time_func.location = span::null();
    time_func.return_type = type::i8_type(1);
    time_func.return_type.is_const = true;
    time_func.is_public = false;
    time_func.is_extern = true;

    auto& this_domain = ctx.get_domain(ctx.this_file);
    this_domain.functions.insert({"__time__", time_func});
}

void regist_pass::regist_single_import(ast::use_stmt* node) {
    if (node->get_module_path().empty()) {
        rp.report(node, "must import at least one symbol from this module.");
        return;
    }
    auto mp = std::string("");
    for (auto i : node->get_module_path()) {
        mp += i->get_name();
        if (i!=node->get_module_path().back()) {
            mp += "::";
        }
    }

    const auto& file = package_manager::singleton()->get_file_name(mp);
    if (file.empty()) {
        rp.report(node, "cannot find module \"" + mp + "\".");
        return;
    }

    auto pkgman = package_manager::singleton();
    if (pkgman->get_analyse_status(file)==package_manager::status::analysing) {
        rp.report(node, "module \"" + mp +
            "\" is not totally analysed, maybe encounter circular import."
        );
        return;
    }
    if (pkgman->get_analyse_status(file)==package_manager::status::not_used) {
        pkgman->set_analyse_status(file, package_manager::status::analysing);
        lexer lex(err);
        parse par(err);
        semantic sema(err);
        mir::ast2mir ast2mir(err, sema.get_context());
        if (lex.scan(file).geterr()) {
            pkgman->set_analyse_status(file, package_manager::status::analysed);
            return;
        }
        if (par.analyse(lex.result()).geterr()) {
            pkgman->set_analyse_status(file, package_manager::status::analysed);
            return;
        }
        if (sema.analyse(par.get_result()).geterr()) {
            pkgman->set_analyse_status(file, package_manager::status::analysed);
            return;
        }
        pkgman->set_analyse_status(file, package_manager::status::analysed);
        // generate mir
        if (ast2mir.generate(par.get_result()).geterr()) {
            rp.report(node,
                "error ocurred when generating mir for module \"" + mp + "\"."
            );
            return;
        }
    }

    if (!ctx.global.domain.count(file)) {
        return;
    }

    const auto& domain = ctx.get_domain(file);
    if (node->get_import_symbol().empty()) {
        for (const auto& i : domain.structs) {
            if (!i.second.is_public) {
                continue;
            }
            import_global_symbol(node, i.second.name,
                {sym_kind::struct_kind, file, i.second.is_public},
                false
            );
        }
        for (const auto& i : domain.generic_structs) {
            if (!i.second.is_public) {
                continue;
            }
            import_global_symbol(node, i.second.name,
                {sym_kind::struct_kind, file, i.second.is_public},
                true
            );
        }
        for (const auto& i : domain.functions) {
            if (!i.second.is_public) {
                continue;
            }
            import_global_symbol(node, i.second.name,
                {sym_kind::func_kind, file, i.second.is_public},
                false
            );
        }
        for (const auto& i : domain.generic_functions) {
            if (!i.second.is_public) {
                continue;
            }
            import_global_symbol(node, i.second.name,
                {sym_kind::func_kind, file, i.second.is_public},
                true
            );
        }
        for (const auto& i : domain.enums) {
            if (!i.second.is_public) {
                continue;
            }
            import_global_symbol(node, i.second.name,
                {sym_kind::enum_kind, file, i.second.is_public},
                false
            );
        }
        return;
    }
    // specified import
    for (auto i : node->get_import_symbol()) {
        if (domain.structs.count(i->get_name()) ||
            domain.generic_structs.count(i->get_name())) {
            if (!check_is_public_struct(i, domain)) {
                continue;
            }
            import_global_symbol(i, i->get_name(),
                {sym_kind::struct_kind, file, true},
                domain.generic_structs.count(i->get_name())
            );
            continue;
        }
        if (domain.functions.count(i->get_name()) ||
            domain.generic_functions.count(i->get_name())) {
            if (!check_is_public_func(i, domain)) {
                continue;
            }
            import_global_symbol(i, i->get_name(),
                {sym_kind::func_kind, file, true},
                domain.generic_functions.count(i->get_name())
            );
            continue;
        }
        if (domain.enums.count(i->get_name())) {
            if (!check_is_public_enum(i, domain)) {
                continue;
            }
            import_global_symbol(i, i->get_name(),
                {sym_kind::enum_kind, file, true},
                false
            );
            continue;
        }
        rp.report(i, "cannot find symbol \"" + i->get_name() +
            "\" in module \"" + mp + "\"."
        );
    }
}

void regist_pass::regist_imported_types(ast::root* node) {
    for (auto i : node->get_use_stmts()) {
        regist_single_import(i);
    }
}

void regist_pass::regist_enums(ast::root* node) {
    for (auto i : node->get_decls()) {
        if (!i->is(ast_type::ast_enum_decl)) {
            continue;
        }
        auto enum_decl_node = reinterpret_cast<ast::enum_decl*>(i);
        regist_single_enum(enum_decl_node);
    }
}

void regist_pass::regist_single_enum(ast::enum_decl* node) {
    const auto& name = node->get_name()->get_name();
    if (ctx.global_symbol().count(name)) {
        rp.report(node, "\"" + name + "\" conflicts with exist symbol.");
        return;
    }

    ctx.get_domain(ctx.this_file).enums.insert({name, {}});
    ctx.insert(name, symbol_info {sym_kind::enum_kind, ctx.this_file, true}, false);

    auto& self = ctx.get_domain(ctx.this_file).enums.at(name);
    self.name = name;
    self.location = node->get_location();
    if (node->is_public_enum()) {
        self.is_public = true;
    }

    // with specified member with number or not
    bool has_specified_member = false;
    bool has_non_specified_member = false;
    for (const auto& i : node->get_member()) {
        if (!i.value) {
            has_non_specified_member = true;
        } else {
            has_specified_member = true;
        }
    }
    if (has_specified_member && has_non_specified_member) {
        rp.report(node, "enum members cannot be both specified and non-specified with number.");
        return;
    }

    for (const auto& i : node->get_member()) {
        if (!i.value) {
            continue;
        }
        if (!check_is_valid_enum_member(i.value)) {
            rp.report(i.value, "enum member cannot be specified with float.");
            return;
        }
    }

    for (const auto& i : node->get_member()) {
        if (self.members.count(i.name->get_name())) {
            rp.report(i.name, "enum member already exists");
            continue;
        }
        const auto& member_name = i.name->get_name();
        if (i.value) {
            self.members[member_name] = (usize)str_to_num(i.value->get_number().c_str());
        } else {
            self.members[member_name] = self.ordered_member.size();
        }
        self.ordered_member.push_back(member_name);
    }
}

void regist_pass::regist_complex_structs(ast::root* node) {
    // regist symbol into symbol table first
    for (auto i : node->get_decls()) {
        if (i->is(ast_type::ast_struct_decl)) {
            auto struct_decl_node = reinterpret_cast<ast::struct_decl*>(i);
            regist_single_struct_symbol(struct_decl_node);
        } else if (i->is(ast_type::ast_tagged_union_decl)) {
            auto union_decl_node = reinterpret_cast<ast::tagged_union_decl*>(i);
            regist_single_tagged_union_symbol(union_decl_node);
        }
    }
    // load field into struct
    for (auto i : node->get_decls()) {
        if (i->is(ast_type::ast_struct_decl)) {
            auto struct_decl_node = reinterpret_cast<ast::struct_decl*>(i);
            regist_single_struct_field(struct_decl_node);
        } else if (i->is(ast_type::ast_tagged_union_decl)) {
            auto union_decl_node = reinterpret_cast<ast::tagged_union_decl*>(i);
            regist_single_tagged_union_member(union_decl_node);
        }
    }
    // do self reference check
    check_struct_self_reference();
}

void regist_pass::regist_single_struct_symbol(ast::struct_decl* node) {
    const auto& name = node->get_name();
    if (ctx.global_symbol().count(name)) {
        rp.report(node, "\"" + name + "\" conflicts with exist symbol.");
        return;
    }

    auto& this_domain = ctx.get_domain(ctx.this_file);

    // insert to global symbol table and domain
    ctx.insert(
        name,
        symbol_info {sym_kind::struct_kind, ctx.this_file, true},
        node->get_generic_types()
    );
    if (node->get_generic_types()) {
        this_domain.generic_structs.insert({name, {}});
    } else {
        this_domain.structs.insert({name, {}});
    }

    auto& self = node->get_generic_types()
        ? this_domain.generic_structs.at(name)
        : this_domain.structs.at(name);
    self.name = name;
    self.location = node->get_location();
    if (node->is_public_struct()) {
        self.is_public = true;
    }
    if (node->is_extern_struct()) {
        self.is_extern = true;
    }
    if (node->get_generic_types()) {
        if (node->get_generic_types()->get_types().empty()) {
            rp.report(node, "generic struct \"" + name +
                "\" must have at least one generic type."
            );
        }
        std::unordered_set<std::string> used_generic;
        for (auto i : node->get_generic_types()->get_types()) {
            const auto& generic_name = i->get_name()->get_name();
            if (ctx.global_symbol().count(generic_name)) {
                rp.report(i, "generic type \"" + generic_name +
                    "\" conflicts with exist symbol."
                );
            }
            if (used_generic.count(generic_name)) {
                rp.report(i, "generic type \"" + generic_name +
                    "\" conflicts with exist generic type."
                );
            }
            used_generic.insert(generic_name);
            self.generic_template.push_back(generic_name);
        }
        self.generic_struct_decl = node->clone();
    }
}

void regist_pass::regist_single_struct_field(ast::struct_decl* node) {
    const auto& name = node->get_name();
    auto& this_domain = ctx.get_domain(ctx.this_file);
    if (!this_domain.structs.count(name) &&
        !this_domain.generic_structs.count(name)) {
        // this branch means the symbol is not loaded successfully
        return;
    }

    auto& self = node->get_generic_types()
        ? this_domain.generic_structs.at(name)
        : this_domain.structs.at(name);

    // initializer generic if needed for field analysis
    ctx.generics = {};
    if (node->get_generic_types()) {
        for (auto i : node->get_generic_types()->get_types()) {
            ctx.generics.insert(i->get_name()->get_name());
        }
    }

    // load fields
    for (auto i : node->get_fields()) {
        auto type_node = i->get_type();
        auto field_type = tr.resolve(type_node);
        if (field_type.is_error()) {
            continue;
        }
        if (self.field.count(i->get_name()->get_name())) {
            rp.report(i, "field name already exists");
        }
        self.field.insert({i->get_name()->get_name(), field_type});
        self.ordered_field.push_back(i->get_name()->get_name());
    }

    auto struct_self_type = type {
        .name = name,
        .loc_file = node->get_file()
    };
    if (node->get_generic_types()) {
        auto& g = struct_self_type.generics;
        for (auto i : node->get_generic_types()->get_types()) {
            g.push_back(tr.resolve(i));
        }
    }

    // add built-in static methods
    self.static_method.insert(
        {"__size__", builtin_struct_size(node->get_location())}
    );
    self.static_method.insert(
        {"__alloc__", builtin_struct_alloc(
            node->get_location(),
            struct_self_type.get_pointer_copy()
        )}
    );
    // add built-in methods
    self.method.insert(
        {"__ptr__", builtin_struct_ptr(
            node->get_location(),
            struct_self_type.get_pointer_copy()
        )}
    );
}

void regist_pass::check_struct_self_reference() {
    const auto& structs = ctx.get_domain(ctx.this_file).structs;

    std::vector<std::string> need_check = {};
    for (const auto& st : structs) {
        for (const auto& field : st.second.field) {
            if (!field.second.is_pointer() &&
                structs.count(field.second.name)) {
                need_check.push_back(st.first);
                break;
            }
        }
    }
    for (const auto& st : need_check) {
        std::queue<std::pair<std::string, std::string>> bfs;
        std::unordered_set<std::string> visited;
        bfs.push({st, st});
        while (!bfs.empty()) {
            auto [cur, path] = bfs.front();
            bfs.pop();
            if (visited.count(cur)) {
                continue;
            } else {
                visited.insert(cur);
            }

            for (const auto& field : structs.at(cur).field) {
                if (field.second.is_pointer()) {
                    continue;
                }
                const auto& type_name = field.second.name;
                auto new_path = path + "::" + field.first + " -> " + type_name;
                if (type_name == st) {
                    rp.report(structs.at(st).location,
                        "self reference detected: " + new_path + "."
                    );
                } else if (structs.count(type_name)) {
                    bfs.push({type_name, new_path});
                }
            }
        }
    }
}

void regist_pass::check_ref_enum(ast::tagged_union_decl* node,
                                 colgm_tagged_union& un) {
    if (node->get_ref_enum_name().empty()) {
        return;
    }

    if (!ctx.global_symbol().count(node->get_ref_enum_name())) {
        rp.report(node, "enum \"" + node->get_ref_enum_name() +
            "\" does not exist, make sure the enum is defined or imported."
        );
        return;
    }

    const auto& info = ctx.global_symbol().at(node->get_ref_enum_name());
    if (info.kind != sym_kind::enum_kind) {
        rp.report(node, "\"" + node->get_ref_enum_name() +
            "\" is not enum."
        );
        return;
    }

    auto ty = type {
        .name = node->get_ref_enum_name(),
        .loc_file = info.loc_file
    };
    ty.is_enum = true;
    un.ref_enum_type = ty;

    const auto& dm = ctx.get_domain(info.loc_file);
    const auto& em = dm.enums.at(node->get_ref_enum_name());

    for (auto i : node->get_members()) {
        if (!em.members.count(i->get_name()->get_name())) {
            rp.report(i, "enum \"" + ty.full_path_name() +
                "\" does not have member \"" + i->get_name()->get_name() + "\"."
            );
        }
    }
}

void regist_pass::load_tagged_union_member_map(ast::tagged_union_decl* node,
                                               colgm_tagged_union& un) {
    if (node->get_ref_enum_name().empty()) {
        for (auto& m : un.ordered_member) {
            un.member_int_map.insert({m, un.member_int_map.size()});
        }
        return;
    }

    if (!ctx.global_symbol().count(node->get_ref_enum_name())) {
        return;
    }

    const auto& info = ctx.global_symbol().at(node->get_ref_enum_name());
    if (info.kind != sym_kind::enum_kind) {
        return;
    }

    const auto& dm = ctx.get_domain(info.loc_file);
    const auto& em = dm.enums.at(node->get_ref_enum_name());

    for (auto i : node->get_members()) {
        if (!em.members.count(i->get_name()->get_name())) {
            continue;
        }
        un.member_int_map.insert({
            i->get_name()->get_name(),
            em.members.at(i->get_name()->get_name())
        });
    }
}

void regist_pass::regist_single_tagged_union_symbol(ast::tagged_union_decl* node) {
    const auto& name = node->get_name();
    if (ctx.global_symbol().count(name)) {
        rp.report(node, "\"" + name + "\" conflicts with exist symbol.");
        return;
    }

    auto& this_domain = ctx.get_domain(ctx.this_file);

    // insert to global symbol table and domain
    ctx.insert(
        name,
        symbol_info {sym_kind::tagged_union_kind, ctx.this_file, true},
        false
    );
    this_domain.tagged_unions.insert({name, {}});

    auto& self = this_domain.tagged_unions.at(name);
    self.name = name;
    self.location = node->get_location();
    if (node->is_public_union()) {
        self.is_public = true;
    }
    if (node->is_extern_union()) {
        self.is_extern = true;
    }
    check_ref_enum(node, self);
}

void regist_pass::regist_single_tagged_union_member(ast::tagged_union_decl* node) {
    const auto& name = node->get_name();
    auto& this_domain = ctx.get_domain(ctx.this_file);
    if (!this_domain.tagged_unions.count(name)) {
        // this branch means the symbol is not loaded successfully
        return;
    }

    auto& self = this_domain.tagged_unions.at(name);
    // load members
    for (auto i : node->get_members()) {
        auto type_node = i->get_type();
        auto member_type = tr.resolve(type_node);
        if (member_type.is_error()) {
            continue;
        }
        if (self.member.count(i->get_name()->get_name())) {
            rp.report(i, "member name already exists");
            continue;
        }
        self.member.insert({i->get_name()->get_name(), member_type});
        self.ordered_member.push_back(i->get_name()->get_name());
    }
    load_tagged_union_member_map(node, self);
}

void regist_pass::regist_global_funcs(ast::root* node) {
    for (auto i : node->get_decls()) {
        if (i->get_ast_type() != ast_type::ast_func_decl) {
            continue;
        }
        auto func_decl = static_cast<ast::func_decl*>(i);
        regist_single_global_func(func_decl);
    }
}

void regist_pass::regist_single_global_func(ast::func_decl* node) {
    const auto& name = node->get_name();
    if (ctx.global_symbol().count(name)) {
        rp.report(node, "\"" + name + "\" conflicts with exist symbol.");
        return;
    }

    auto& this_domain = ctx.get_domain(ctx.this_file);
    if (this_domain.functions.count(name)) {
        rp.report(node, "function \"" + name + "\" conflicts with exist symbol.");
        return;
    }

    // insert into symbol table
    ctx.insert(
        name,
        symbol_info {sym_kind::func_kind, ctx.this_file, true},
        node->get_generic_types()
    );

    // generate data structure
    if (node->get_generic_types()) {
        this_domain.generic_functions.insert({
            name,
            generate_single_global_func(node)
        });
    } else {
        this_domain.functions.insert({
            name,
            generate_single_global_func(node)
        });
    }

    auto& self = this_domain.functions.count(name)
        ? this_domain.functions.at(name)
        : this_domain.generic_functions.at(name);
    if (node->is_public_func()) {
        self.is_public = true;
    }
    if (node->is_extern_func()) {
        self.is_extern = true;
    }
    if (node->get_generic_types()) {
        if (node->get_generic_types()->get_types().empty()) {
            rp.report(node, "generic function \"" + name +
                "\" must have at least one generic type."
            );
        }
        if (node->is_extern_func()) {
            rp.report(node, "extern generic function \"" + name +
                "\" is not supported."
            );
        }
        std::unordered_set<std::string> used_generic;
        for (auto i : node->get_generic_types()->get_types()) {
            const auto& generic_name = i->get_name()->get_name();
            if (ctx.global_symbol().count(generic_name)) {
                rp.report(i, "generic type \"" + generic_name +
                    "\" conflicts with exist symbol."
                );
            }
            if (used_generic.count(generic_name)) {
                rp.report(i, "generic type \"" + generic_name +
                    "\" conflicts with exist generic type."
                );
            }
            used_generic.insert(generic_name);
            self.generic_template.push_back(i->get_name()->get_name());
        }
        // copy the node for template expansion
        self.generic_func_decl = node->clone();
    }
}

colgm_func regist_pass::generate_single_global_func(func_decl* node) {
    auto func_self = colgm_func();
    func_self.name = node->get_name();
    func_self.location = node->get_location();

    // create generic temporary table
    ctx.generics = {};
    if (node->get_generic_types()) {
        for (auto i : node->get_generic_types()->get_types()) {
            ctx.generics.insert(i->get_name()->get_name());
        }
    }

    generate_parameter_list(node->get_params(), func_self);
    generate_return_type(node->get_return_type(), func_self);
    if (node->get_name()=="main" && func_self.return_type.is_void()) {
        rp.warn(node, "main function should return integer.");
    }

    // clear generic temporary table
    ctx.generics = {};

    return func_self;
}

void regist_pass::generate_parameter_list(param_list* node, colgm_func& self) {
    for (auto i : node->get_params()) {
        generate_parameter(i, self);
    }
}

void regist_pass::generate_parameter(param* node, colgm_func& self) {
    const auto& name = node->get_name()->get_name();
    if (self.find_parameter(name)) {
        rp.report(node->get_name(),
            "redefinition of parameter \"" + name + "\"."
        );
        return;
    }
    self.add_parameter(name, tr.resolve(node->get_type()));
}

void regist_pass::generate_return_type(type_def* node, colgm_func& self) {
    self.return_type = tr.resolve(node);
}

void regist_pass::regist_impls(ast::root* node) {
    for (auto i : node->get_decls()) {
        if (i->get_ast_type() != ast_type::ast_impl) {
            continue;
        }
        auto impl_decl = reinterpret_cast<ast::impl_struct*>(i);
        regist_single_impl(impl_decl);
    }
}

void regist_pass::regist_single_impl(ast::impl_struct* node) {
    auto& dm = ctx.get_domain(ctx.this_file);
    if (!dm.structs.count(node->get_struct_name()) &&
        !dm.generic_structs.count(node->get_struct_name())) {
        rp.report(node, "undefined struct \"" + node->get_struct_name() + "\".");
        return;
    }
    auto& stct = dm.structs.count(node->get_struct_name())
        ? dm.structs.at(node->get_struct_name())
        : dm.generic_structs.at(node->get_struct_name());

    ctx.generics = {};
    for (const auto& i : stct.generic_template) {
        ctx.generics.insert(i);
    }
    for (auto i : node->get_methods()) {
        const auto name = i->get_monomorphic_name();
        if (stct.field.count(name)) {
            rp.report(i, "conflict with field \"" + name + "\".");
            continue;
        }
        if (stct.method.count(name) || stct.static_method.count(name)) {
            rp.report(i, "method \"" + name + "\" already exists.");
            continue;
        }

        // generic methods are not allowed
        if (i->get_generic_types()) {
            rp.report(i, "method \"" + name + "\" cannot be generic.");
        }
        auto func = generate_method(i, stct);
        if (i->is_public_func()) {
            func.is_public = true;
        }
        if (i->is_extern_func()) {
            rp.report(i, "extern method is not supported.");
        }
        if (func.ordered_params.size() && func.ordered_params.front() == "self") {
            stct.method.insert({name, func});
        } else {
            stct.static_method.insert({name, func});
        }
    }

    if (node->get_generic_types()) {
        if (node->get_generic_types()->get_types().empty()) {
            rp.report(node, "generic impl must have at least one generic type.");
            return;
        }
        const auto& impl_generic_vec = node->get_generic_types()->get_types();
        if (stct.generic_template.size() != impl_generic_vec.size()) {
            rp.report(node, "generic type count does not match.");
            return;
        }
        for (u64 i = 0; i < stct.generic_template.size(); ++i) {
            const auto& name = impl_generic_vec[i]->get_name()->get_name();
            if (stct.generic_template[i] != name) {
                rp.report(impl_generic_vec[i], "generic type \"" + name +
                    "\" does not match with \"" +
                    stct.generic_template[i] + "\"."
                );
            }
        }
        // copy ast of this impl struct for template expansion
        stct.generic_struct_impl.push_back(node->clone());
    }
}

colgm_func regist_pass::generate_method(ast::func_decl* node,
                                        const colgm_struct& stct) {
    auto func_self = colgm_func();
    func_self.name = node->get_monomorphic_name();
    func_self.location = node->get_location();
    generate_method_parameter_list(node->get_params(), func_self, stct);
    generate_return_type(node->get_return_type(), func_self);
    return func_self;
}

void regist_pass::generate_self_parameter(ast::param* node,
                                          const colgm_struct& stct) {
    auto new_type_def = new type_def(node->get_location());
    new_type_def->set_name(new identifier(
        node->get_name()->get_location(),
        stct.name
    ));
    if (stct.generic_template.size()) {
        new_type_def->set_generic_types(
            new generic_type_list(node->get_location())
        );
        for (auto& i : stct.generic_template) {
            auto t = new type_def(node->get_location());
            t->set_name(new identifier(node->get_location(), i));
            new_type_def->get_generic_types()->add_type(t);
        }
    }
    new_type_def->add_pointer_level();
    node->set_type(new_type_def);
}

void regist_pass::generate_method_parameter_list(param_list* node,
                                                 colgm_func& self,
                                                 const colgm_struct& stct) {
    for (auto i : node->get_params()) {
        bool is_self = i->get_name()->get_name()=="self";
        if (is_self && i!=node->get_params().front()) {
            rp.report(i->get_name(), "\"self\" must be the first parameter.");
        }
        if (is_self && i->get_type()) {
            rp.warn(i->get_type(), "\"self\" does not need type.");
        }
        if (is_self && !i->get_type()) {
            generate_self_parameter(i, stct);
        }
        generate_parameter(i, self);
    }

    if (self.ordered_params.empty() ||
        self.ordered_params.front() != "self") {
        return;
    }

    // we still need to check self type, for user may specify a wrong type
    // check self type is the pointer of implemented struct
    const auto& self_type = self.params.at(self.ordered_params.front());
    if (self_type.name != stct.name ||
        self_type.loc_file != stct.location.file ||
        self_type.pointer_depth != 1) {
        rp.report(node->get_params().front(),
            "\"self\" should be \"" + stct.name + "*\", but get \"" +
            self_type.to_string() + "\"."
        );
    }
}

void regist_pass::run(ast::root* ast_root) {
    regist_primitive_types();

    ctx.get_domain(ctx.this_file).enums.clear();
    ctx.get_domain(ctx.this_file).structs.clear();
    ctx.get_domain(ctx.this_file).tagged_unions.clear();
    ctx.get_domain(ctx.this_file).generic_structs.clear();
    ctx.get_domain(ctx.this_file).functions.clear();
    ctx.get_domain(ctx.this_file).generic_functions.clear();

    regist_builtin_funcs();
    regist_imported_types(ast_root);
    if (err.geterr()) {
        return;
    }

    regist_enums(ast_root);
    regist_complex_structs(ast_root);
    if (err.geterr()) {
        return;
    }

    regist_global_funcs(ast_root);
    if (err.geterr()) {
        return;
    }

    regist_impls(ast_root);
    if (err.geterr()) {
        return;
    }

    gnv.scan_and_insert(ast_root);
}

}