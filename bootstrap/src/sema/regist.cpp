#include "ast/stmt.h"
#include "ast/expr.h"
#include "sema/regist.h"
#include "lexer.h"
#include "parse.h"
#include "sema/semantic.h"
#include "mir/ast2mir.h"

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
}

}