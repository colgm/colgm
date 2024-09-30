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

void regist_pass::run(ast::root* ast_root) {
    regist_basic_types();
    regist_imported_types(ast_root);
    if (err.geterr()) {
        return;
    }

    ctx.global.domain.at(ctx.this_file).enums.clear();
    ctx.global.domain.at(ctx.this_file).structs.clear();
    ctx.global.domain.at(ctx.this_file).functions.clear();
}

}