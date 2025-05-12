#include "ast/stmt.h"
#include "ast/expr.h"
#include "sema/regist_pass.h"
#include "lexer.h"
#include "parse.h"
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
            for(const auto& i : select_type.generics) {
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
        ? ctx.global.domain.at(type_node->get_redirect_location())
        : ctx.global.domain.at(type_node->get_file());

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
    const auto& sdm = ctx.global.domain.at(sym.loc_file);
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
    auto unique_name = sym.loc_file + "|" + ss.str();
    if (generic_type_map.count(unique_name)) {
        return;
    }
    generic_type_map.insert({unique_name, {}});
    auto& rec = generic_type_map.at(unique_name);
    rec.name = ss.str();
    rec.generic_type.name = type_name;
    rec.generic_type.loc_file = sym.loc_file;

    for(i64 i = 0; i < generic_template.size(); ++i) {
        auto t = type_list[i];
        const auto resolved_type = tr.resolve(t);
        rec.generic_type.generics.push_back(resolved_type);
        rec.types.insert({generic_template[i], resolved_type});
    }
}

bool generic_visitor::visit_struct_decl(struct_decl* node) {
    // do not scan generic struct block
    if (node->get_generic_types()) {
        return true;
    }
    for(auto i : node->get_fields()) {
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
    for(auto i : node->get_params()->get_params()) {
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
    for(auto i : node->get_methods()) {
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
        ? ctx.global.domain.at(node->get_redirect_location())
        : ctx.global.domain.at(node->get_file());
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
    const auto& sdm = ctx.global.domain.at(sym.loc_file);
    const auto& generic_template = sym.kind == sym_kind::struct_kind
        ? sdm.generic_structs.at(type_name).generic_template
        : sdm.generic_functions.at(type_name).generic_template;
    const auto& type_list = node->get_generic_types()->get_types();
    check_generic_type(node, type_name, sym, type_list, generic_template);
    return true;
}

bool generic_visitor::visit_type_def(ast::type_def* node) {
    if (node->get_generic_types()) {
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
    for(auto& g : t.generics) {
        replace_type(g, data);
    }
}

void generic_visitor::replace_struct_type(colgm_struct& s,
                                          const generic_data& data) {
    for(auto& i : s.field) {
        replace_type(i.second.symbol_type, data);
    }
    for(auto& i : s.ordered_field) {
        replace_type(i.symbol_type, data);
    }

    for(auto& i : s.method) {
        replace_func_type(i.second, data);
    }
    for(auto& i : s.static_method) {
        replace_func_type(i.second, data);
    }

    type_replace_pass trp(err, ctx, data);
    if (s.generic_struct_decl) {
        trp.visit_struct(s.generic_struct_decl);
        root->add_decl(s.generic_struct_decl);
        // s.name is generated with generic
        // for example original name is "foo",
        // but now it should be replaced with "foo<int, bool>"
        s.generic_struct_decl->set_name(s.name);
        s.generic_struct_decl->clear_generic_types();
        s.generic_struct_decl = nullptr;
    }
    for(auto i : s.generic_struct_impl) {
        trp.visit_impl(i);
        root->add_decl(i);
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
    for(auto& i : f.parameters) {
        replace_type(i.symbol_type, data);
    }

    if (f.generic_func_decl) {
        type_replace_pass trp(err, ctx, data);
        trp.visit_func(f.generic_func_decl);
        root->add_decl(f.generic_func_decl);
        // f.name is generated with generic
        // for example original name is "foo",
        // but now it should be replaced with "foo<int, bool>"
        f.generic_func_decl->set_name(f.name);
        f.generic_func_decl->clear_generic_types();
    }
    f.generic_func_decl = nullptr;
}

void generic_visitor::report_recursive_generic_generation() {
    for(const auto& i : generic_type_map) {
        const auto& data = i.second.generic_type;
        if (!ctx.global.domain.count(data.loc_file)) {
            continue;
        }
        const auto& dm = ctx.global.domain.at(data.loc_file);
        const auto generic_name = data.generic_name();
        if (dm.structs.count(generic_name) || dm.functions.count(generic_name)) {
            continue;
        }

        const auto& loc = dm.generic_structs.count(data.name)
            ? dm.generic_structs.at(data.name).location
            : dm.generic_functions.at(data.name).location;

        auto info = std::string("template instantiation depth exceeds maximum of ");
        info += std::to_string(MAX_RECURSIVE_DEPTH) + ":\n";
        info += "  --> " + generic_name;
        err.err(loc, info);
    }
}

void generic_visitor::dump() const {
    for(const auto& i : generic_type_map) {
        std::cout << i.second.generic_type.full_path_name();
        std::cout << ": " << i.second.generic_type.loc_file << "\n";
        for(const auto& real : i.second.generic_type.generics) {
            std::cout << "  ";
            std::cout << real.full_path_name() << " ";
            std::cout << real.loc_file << "\n";
        }
        std::cout << "\n";
    }
}

u64 generic_visitor::insert_into_symbol_table() {
    ++visit_count;
    if (visit_count > MAX_RECURSIVE_DEPTH) {
        report_recursive_generic_generation();
        return 0;
    }
    u64 insert_count = 0;
    for(const auto& i : generic_type_map) {
        const auto& data = i.second.generic_type;
        // not full path name
        const auto generic_name = data.generic_name();

        if (!ctx.global.domain.count(data.loc_file)) {
            continue;
        }

        auto& dm = ctx.global.domain.at(data.loc_file);
        if (dm.generic_structs.count(data.name)) {
            // no need to load again, otherwise will cause redefine error
            if (dm.structs.count(generic_name)) {
                continue;
            }
            ++insert_count;
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
            replace_struct_type(dm.structs.at(generic_name), i.second);
        } else if (dm.generic_functions.count(data.name)) {
            // no need to load again, otherwise will cause redefine error
            if (dm.functions.count(generic_name)) {
                continue;
            }
            ++insert_count;
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
            replace_func_type(dm.functions.at(generic_name), i.second);
        }
    }
    return insert_count;
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
    if (ctx.generic_symbol().count(name)) {
        const auto& info = ctx.generic_symbol().at(name);
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
    func.parameters.push_back(symbol {.name = "self", .symbol_type = ty});
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
    for(auto i : primitives) {
        ctx.global_symbol().insert({i, {sym_kind::basic_kind, "", true}});
    }

    // load default methods
    for(auto i : primitives) {
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

void regist_pass::regist_single_import(ast::use_stmt* node) {
    if (node->get_module_path().empty()) {
        rp.report(node, "must import at least one symbol from this module.");
        return;
    }
    auto mp = std::string("");
    for(auto i : node->get_module_path()) {
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

    const auto& domain = ctx.global.domain.at(file);
    if (node->get_import_symbol().empty()) {
        for(const auto& i : domain.structs) {
            import_global_symbol(node, i.second.name,
                {sym_kind::struct_kind, file, i.second.is_public},
                false
            );
        }
        for(const auto& i : domain.generic_structs) {
            import_global_symbol(node, i.second.name,
                {sym_kind::struct_kind, file, i.second.is_public},
                true
            );
        }
        for(const auto& i : domain.functions) {
            import_global_symbol(node, i.second.name,
                {sym_kind::func_kind, file, i.second.is_public},
                false
            );
        }
        for(const auto& i : domain.generic_functions) {
            import_global_symbol(node, i.second.name,
                {sym_kind::func_kind, file, i.second.is_public},
                true
            );
        }
        for(const auto& i : domain.enums) {
            import_global_symbol(node, i.second.name,
                {sym_kind::enum_kind, file, i.second.is_public},
                false
            );
        }
        return;
    }
    // specified import
    for(auto i : node->get_import_symbol()) {
        if (domain.structs.count(i->get_name()) ||
            domain.generic_structs.count(i->get_name())) {
            import_global_symbol(i, i->get_name(),
                {sym_kind::struct_kind, file, check_is_public_struct(i, domain)},
                domain.generic_structs.count(i->get_name())
            );
            continue;
        }
        if (domain.functions.count(i->get_name()) ||
            domain.generic_functions.count(i->get_name())) {
            import_global_symbol(i, i->get_name(),
                {sym_kind::func_kind, file, check_is_public_func(i, domain)},
                domain.generic_functions.count(i->get_name())
            );
            continue;
        }
        if (domain.enums.count(i->get_name())) {
            import_global_symbol(i, i->get_name(),
                {sym_kind::enum_kind, file, check_is_public_enum(i, domain)},
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
    for(auto i : node->get_use_stmts()) {
        regist_single_import(i);
    }
}

void regist_pass::regist_enums(ast::root* node) {
    for(auto i : node->get_decls()) {
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
    if (ctx.global.domain.at(ctx.this_file).enums.count(name)) {
        rp.report(node, "enum \"" + name + "\" conflicts with exist symbol.");
        return;
    }
    ctx.global.domain.at(ctx.this_file).enums.insert({name, {}});
    ctx.insert(name, {sym_kind::enum_kind, ctx.this_file, true}, false);

    auto& self = ctx.global.domain.at(ctx.this_file).enums.at(name);
    self.name = name;
    self.location = node->get_location();
    if (node->is_public_enum()) {
        self.is_public = true;
    }

    // with specified member with number or not
    bool has_specified_member = false;
    bool has_non_specified_member = false;
    for(const auto& i : node->get_member()) {
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

    for(const auto& i : node->get_member()) {
        if (!i.value) {
            continue;
        }
        if (!check_is_valid_enum_member(i.value)) {
            rp.report(i.value, "enum member cannot be specified with float.");
            return;
        }
    }

    for(const auto& i : node->get_member()) {
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

void regist_pass::regist_structs(ast::root* node) {
    // regist symbol into symbol table first
    for(auto i : node->get_decls()) {
        if (!i->is(ast_type::ast_struct_decl)) {
            continue;
        }
        auto struct_decl_node = reinterpret_cast<ast::struct_decl*>(i);
        regist_single_struct_symbol(struct_decl_node);
    }
    // load field into struct
    for(auto i : node->get_decls()) {
        if (!i->is(ast_type::ast_struct_decl)) {
            continue;
        }
        auto struct_decl_node = reinterpret_cast<ast::struct_decl*>(i);
        regist_single_struct_field(struct_decl_node);
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

    auto& this_domain = ctx.global.domain.at(ctx.this_file);
    if (this_domain.structs.count(name) ||
        this_domain.generic_structs.count(name)) {
        rp.report(node, "struct \"" + name + "\" conflicts with exist symbol.");
    }

    // insert to global symbol table and domain
    ctx.insert(
        name,
        {sym_kind::struct_kind, ctx.this_file, true},
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
        for(auto i : node->get_generic_types()->get_types()) {
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
    auto& this_domain = ctx.global.domain.at(ctx.this_file);
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
        for(auto i : node->get_generic_types()->get_types()) {
            ctx.generics.insert(i->get_name()->get_name());
        }
    }

    // load fields
    for(auto i : node->get_fields()) {
        auto type_node = i->get_type();
        auto field_type = symbol {
            .name = i->get_name()->get_name(),
            .symbol_type = tr.resolve(type_node)
        };
        if (field_type.symbol_type.is_error()) {
            continue;
        }
        if (self.field.count(i->get_name()->get_name())) {
            rp.report(i, "field name already exists");
        }
        self.field.insert({i->get_name()->get_name(), field_type});
        self.ordered_field.push_back(field_type);
    }

    auto struct_self_type = type {
        .name = name,
        .loc_file = node->get_file()
    };
    if (node->get_generic_types()) {
        auto& g = struct_self_type.generics;
        for(auto i : node->get_generic_types()->get_types()) {
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
    const auto& structs = ctx.global.domain.at(ctx.this_file).structs;

    std::vector<std::string> need_check = {};
    for(const auto& st : structs) {
        for(const auto& field : st.second.field) {
            if (!field.second.symbol_type.is_pointer() &&
                structs.count(field.second.symbol_type.name)) {
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
                if (field.second.symbol_type.is_pointer()) {
                    continue;
                }
                const auto& type_name = field.second.symbol_type.name;
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

    auto& this_domain = ctx.global.domain.at(ctx.this_file);
    if (this_domain.functions.count(name)) {
        rp.report(node, "function \"" + name + "\" conflicts with exist symbol.");
        return;
    }

    // insert into symbol table
    ctx.insert(
        name,
        {sym_kind::func_kind, ctx.this_file, true},
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
        for(auto i : node->get_generic_types()->get_types()) {
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
        for(auto i : node->get_generic_types()->get_types()) {
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
    for(auto i : node->get_params()) {
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
    for(auto i : node->get_decls()) {
        if (i->get_ast_type() != ast_type::ast_impl) {
            continue;
        }
        auto impl_decl = reinterpret_cast<ast::impl_struct*>(i);
        regist_single_impl(impl_decl);
    }
}

void regist_pass::regist_single_impl(ast::impl_struct* node) {
    auto& dm = ctx.global.domain.at(ctx.this_file);
    if (!dm.structs.count(node->get_struct_name()) &&
        !dm.generic_structs.count(node->get_struct_name())) {
        rp.report(node, "undefined struct \"" + node->get_struct_name() + "\".");
        return;
    }
    auto& stct = dm.structs.count(node->get_struct_name())
        ? dm.structs.at(node->get_struct_name())
        : dm.generic_structs.at(node->get_struct_name());

    ctx.generics = {};
    for(const auto& i : stct.generic_template) {
        ctx.generics.insert(i);
    }
    for(auto i : node->get_methods()) {
        if (stct.field.count(i->get_name())) {
            rp.report(i, "conflict with field \"" + i->get_name() + "\".");
            continue;
        }
        if (stct.method.count(i->get_name())) {
            rp.report(i, "method \"" + i->get_name() + "\" already exists.");
            continue;
        }

        // generic methods are not allowed
        if (i->get_generic_types()) {
            rp.report(i, "method \"" + i->get_name() + "\" cannot be generic.");
        }
        auto func = generate_method(i, stct);
        if (i->is_public_func()) {
            func.is_public = true;
        }
        if (i->is_extern_func()) {
            rp.report(i, "extern method is not supported.");
        }
        if (func.parameters.size() && func.parameters[0].name=="self") {
            stct.method.insert({i->get_name(), func});
        } else {
            stct.static_method.insert({i->get_name(), func});
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
        for(u64 i = 0; i < stct.generic_template.size(); ++i) {
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
    func_self.name = node->get_name();
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
        for(auto& i : stct.generic_template) {
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
    for(auto i : node->get_params()) {
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

    if (self.parameters.empty() ||
        self.parameters.front().name!="self") {
        return;
    }

    // we still need to check self type, for user may specify a wrong type
    // check self type is the pointer of implemented struct
    const auto& self_type = self.parameters.front().symbol_type;
    if (self_type.name!=stct.name ||
        self_type.loc_file!=stct.location.file ||
        self_type.pointer_depth!=1) {
        rp.report(node->get_params().front(),
            "\"self\" should be \"" + stct.name + "*\", but get \"" +
            self_type.to_string() + "\"."
        );
    }
}

void regist_pass::run(ast::root* ast_root) {
    regist_primitive_types();
    regist_imported_types(ast_root);
    if (err.geterr()) {
        return;
    }

    ctx.global.domain.at(ctx.this_file).enums.clear();
    ctx.global.domain.at(ctx.this_file).structs.clear();
    ctx.global.domain.at(ctx.this_file).generic_structs.clear();
    ctx.global.domain.at(ctx.this_file).functions.clear();
    ctx.global.domain.at(ctx.this_file).generic_functions.clear();

    regist_enums(ast_root);
    regist_structs(ast_root);
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