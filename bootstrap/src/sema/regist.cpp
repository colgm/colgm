#include "ast/stmt.h"
#include "ast/expr.h"
#include "sema/regist.h"
#include "lexer.h"
#include "parse.h"
#include "sema/semantic.h"
#include "mir/ast2mir.h"

#include <cstring>
#include <sstream>
#include <queue>
#include <unordered_map>

namespace colgm {

bool regist_pass::check_is_public_struct(ast::identifier* node,
                                         const colgm_module& domain) {
    if (!domain.structs.count(node->get_name())) {
        return false;
    }
    if (!domain.structs.at(node->get_name()).is_public) {
        report(node,
            "cannot import private struct \"" +
            node->get_name() + "\"."
        );
        return false;
    }
    return true;
}

bool regist_pass::check_is_public_func(ast::identifier* node,
                                       const colgm_module& domain) {
    if (!domain.functions.count(node->get_name())) {
        return false;
    }
    if (!domain.functions.at(node->get_name()).is_public) {
        report(node,
            "cannot import private function \"" +
            node->get_name() + "\"."
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
        report(node,
            "cannot import private enum \"" +
            node->get_name() + "\"."
        );
        return false;
    }
    return true;
}

void regist_pass::import_global_symbol(ast::node* n, 
                                       const std::string& name,
                                       const symbol_info& sym) {
    if (ctx.global_symbol.count(name)) {
        const auto& info = ctx.global_symbol.at(name);
        report(n, "\"" + name +
            "\" conflicts, another declaration is in \"" +
            info.loc_file + "\"."
        );
        return;
    }
    ctx.global_symbol.insert({name, sym});
}

bool regist_pass::check_is_specified_enum_member(ast::number_literal* node) {
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

void regist_pass::regist_basic_types() {
    ctx.global_symbol = {
        {"i64", {sym_kind::basic_kind, "", true}},
        {"i32", {sym_kind::basic_kind, "", true}},
        {"i16", {sym_kind::basic_kind, "", true}},
        {"i8", {sym_kind::basic_kind, "", true}},
        {"u64", {sym_kind::basic_kind, "", true}},
        {"u32", {sym_kind::basic_kind, "", true}},
        {"u16", {sym_kind::basic_kind, "", true}},
        {"u8", {sym_kind::basic_kind, "", true}},
        {"f32", {sym_kind::basic_kind, "", true}},
        {"f64", {sym_kind::basic_kind, "", true}},
        {"void", {sym_kind::basic_kind, "", true}},
        {"bool", {sym_kind::basic_kind, "", true}}
    };
}

void regist_pass::regist_single_import(ast::use_stmt* node) {
    if (node->get_module_path().empty()) {
        report(node, "must import at least one symbol from this module.");
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
        report(node, "cannot find module \"" + mp + "\".");
        return;
    }

    auto pkgman = package_manager::singleton();
    if (pkgman->get_analyse_status(file)==package_manager::status::analysing) {
        report(node, "module \"" + mp +
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
            report(node,
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
                {sym_kind::struct_kind, file, i.second.is_public}
            );
        }
        for(const auto& i : domain.functions) {
            import_global_symbol(node, i.second.name,
                {sym_kind::func_kind, file, i.second.is_public}
            );
        }
        for(const auto& i : domain.enums) {
            import_global_symbol(node, i.second.name,
                {sym_kind::enum_kind, file, i.second.is_public}
            );
        }
        return;
    }
    // specified import
    for(auto i : node->get_import_symbol()) {
        if (domain.structs.count(i->get_name())) {
            import_global_symbol(i, i->get_name(),
                {sym_kind::struct_kind, file, check_is_public_struct(i, domain)}
            );
            continue;
        }
        if (domain.functions.count(i->get_name())) {
            import_global_symbol(i, i->get_name(),
                {sym_kind::func_kind, file, check_is_public_func(i, domain)}
            );
            continue;
        }
        if (domain.enums.count(i->get_name())) {
            import_global_symbol(i, i->get_name(),
                {sym_kind::enum_kind, file, check_is_public_enum(i, domain)}
            );
            continue;
        }
        report(i, "cannot find symbol \"" + i->get_name() +
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
        if (i->get_ast_type()!=ast_type::ast_enum_decl) {
            continue;
        }
        auto enum_decl_node = reinterpret_cast<ast::enum_decl*>(i);
        regist_single_enum(enum_decl_node);
    }
}

void regist_pass::regist_single_enum(ast::enum_decl* node) {
    const auto& name = node->get_name()->get_name();
    if (ctx.global_symbol.count(name)) {
        report(node, "\"" + name + "\" conflicts with exist symbol.");
        return;
    }
    if (ctx.global.domain.at(ctx.this_file).enums.count(name)) {
        report(node, "enum \"" + name + "\" conflicts with exist symbol.");
        return;
    }
    ctx.global.domain.at(ctx.this_file).enums.insert({name, {}});
    ctx.global_symbol.insert({name, {sym_kind::enum_kind, ctx.this_file, true}});

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
        report(node, "enum members cannot be both specified and non-specified with number.");
        return;
    }

    for(const auto& i : node->get_member()) {
        if (!i.value) {
            continue;
        }
        if (!check_is_specified_enum_member(i.value)) {
            report(i.value, "enum member cannot be specified with float number.");
            return;
        }
    }

    for(const auto& i : node->get_member()) {
        if (self.members.count(i.name->get_name())) {
            report(i.name, "enum member already exists");
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
        if (i->get_ast_type()!=ast_type::ast_struct_decl) {
            continue;
        }
        auto struct_decl_node = reinterpret_cast<ast::struct_decl*>(i);
        regist_single_struct_symbol(struct_decl_node);
    }
    // load field into struct
    for(auto i : node->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_struct_decl) {
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
    if (ctx.global_symbol.count(name)) {
        report(node, "\"" + name + "\" conflicts with exist symbol.");
        return;
    }

    auto& this_domain = ctx.global.domain.at(ctx.this_file);
    if (this_domain.structs.count(name) ||
        this_domain.generic_structs.count(name)) {
        report(node, "struct \"" + name + "\" conflicts with exist symbol.");
    }

    // insert to global symbol table and domain
    ctx.global_symbol.insert({name, {sym_kind::struct_kind, ctx.this_file, true}});
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
            report(node, "generic struct \"" + name +
                "\" must have at least one generic type."
            );
        }
        std::unordered_set<std::string> used_generic;
        for(auto i : node->get_generic_types()->get_types()) {
            const auto& generic_name = i->get_name()->get_name();
            if (used_generic.count(generic_name)) {
                report(i, "generic type \"" + generic_name +
                    "\" conflicts with exist generic type."
                );
            }
            used_generic.insert(generic_name);
            self.generic_template.push_back(generic_name);
        }
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

    // initializer generic if needed
    ctx.generics = {};
    if (node->get_generic_types()) {
        for(auto i : node->get_generic_types()->get_types()) {
            ctx.generics.insert(i->get_name()->get_name());
        }
    }

    // load fields
    for(auto i : node->get_fields()) {
        auto type_node = i->get_type();
        auto type_name_node = type_node->get_name();
        const auto& basic_type_name = type_name_node->get_name();
        if (!ctx.global_symbol.count(basic_type_name) &&
            !ctx.generics.count(basic_type_name)) {
            report(type_name_node,
                "undefined type \"" + basic_type_name + "\"."
            );
            continue;
        }
        auto field_type = symbol {
            .name = i->get_name()->get_name(),
            .symbol_type = {
                .name = basic_type_name,
                .loc_file = ctx.global_symbol.count(basic_type_name)
                    ? ctx.global_symbol.at(basic_type_name).loc_file
                    : "",
                .pointer_depth = type_node->get_pointer_level()
            }
        };
        if (self.field.count(i->get_name()->get_name())) {
            report(i, "field name already exists");
        }
        self.field.insert({i->get_name()->get_name(), field_type});
        self.ordered_field.push_back(field_type);
    }

    // add built-in static method size, for malloc usage
    self.static_method.insert(
        {"__size__", builtin_struct_size(node->get_location())}
    );
    self.static_method.insert(
        {"__alloc__", builtin_struct_alloc(node->get_location(), {
            .name = name,
            .loc_file = node->get_file(),
            .pointer_depth = 1
        })}
    );

    // clear
    ctx.generics.clear();
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
    for(const auto& st : need_check) {
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

            for(const auto& field : structs.at(cur).field) {
                if (field.second.symbol_type.is_pointer()) {
                    continue;
                }
                const auto& type_name = field.second.symbol_type.name;
                auto new_path = path + "::" + field.first + " -> " + type_name;
                if (type_name == st) {
                    err.err("semantic", structs.at(st).location,
                        "self reference detected: " + new_path + "."
                    );
                } else if (structs.count(type_name)) {
                    bfs.push({type_name, new_path});
                }
            }
        }
    }
}

void regist_pass::run(ast::root* ast_root) {
    regist_basic_types();
    regist_imported_types(ast_root);
    if (err.geterr()) {
        return;
    }

    ctx.global.domain.at(ctx.this_file).enums.clear();
    ctx.global.domain.at(ctx.this_file).structs.clear();
    ctx.global.domain.at(ctx.this_file).functions.clear();

    regist_enums(ast_root);
    regist_structs(ast_root);
    if (err.geterr()) {
        return;
    }
}

}