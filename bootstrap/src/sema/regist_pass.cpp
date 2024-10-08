#include "ast/stmt.h"
#include "ast/expr.h"
#include "sema/regist_pass.h"
#include "lexer.h"
#include "parse.h"
#include "sema/semantic.h"
#include "mir/ast2mir.h"

#include <cstring>
#include <sstream>
#include <queue>
#include <unordered_map>

namespace colgm {

bool generic_visitor::visit_func_decl(func_decl* node) {
    // do not scan generic function block
    if (node->get_generic_types()) {
        return true;
    }
    node->get_params()->accept(this);
    node->get_return_type()->accept(this);
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

    const auto& type_name = node->get_id()->get_name();
    if (!ctx.global_symbol.count(type_name)) {
        rp.report(node, "unknown type \"" + type_name + "\".");
        return true;
    }
    if (!ctx.generic_symbol.count(type_name)) {
        rp.report(node, "\"" + type_name + "\" is not a generic type.");
        return true;
    }

    const auto& sym = ctx.global_symbol.at(type_name);;
    const auto& dm = ctx.global.domain.at(sym.loc_file);
    const auto& generic_template = sym.kind == sym_kind::struct_kind
        ? dm.generic_structs.at(type_name).generic_template
        : dm.generic_functions.at(type_name).generic_template;

    if (node->get_generic_types()->get_types().size() != generic_template.size()) {
        rp.report(node, "generic type count does not match.");
        return true;
    }

    // generate real name
    std::stringstream ss;
    ss << type_name << "<";
    for (auto i : node->get_generic_types()->get_types()) {
        const auto& name = i->get_name()->get_name();
        const auto type = tr.resolve(i);
        ss << type.full_path_name();
        if (i != node->get_generic_types()->get_types().back()) {
            ss << ",";
        }
    }
    ss << ">";
    if (rp.has_error()) {
        return true;
    }

    // insert data
    if (generic_data_map.count(ss.str())) {
        return true;
    }
    generic_data_map.insert({ss.str(), {}});
    auto& data = generic_data_map.at(ss.str());
    data.name = type_name;
    data.loc_file = sym.loc_file;

    for(i64 i = 0; i < generic_template.size(); ++i) {
        auto t = node->get_generic_types()->get_types()[i];
        data.types.insert({generic_template[i], tr.resolve(t)});
    }
    return true;
}

void generic_visitor::dump() const {
    for(const auto& i : generic_data_map) {
        std::cout << i.first << ": " << i.second.loc_file << "\n";
        for(const auto& real : i.second.types) {
            std::cout << "  " << real.first << ": ";
            std::cout << real.second.full_path_name() << " ";
            std::cout << real.second.loc_file << "\n";
        }
        std::cout << "\n";
    }
}

bool regist_pass::check_is_public_struct(ast::identifier* node,
                                         const colgm_module& domain) {
    if (!domain.structs.count(node->get_name())) {
        return false;
    }
    if (!domain.structs.at(node->get_name()).is_public) {
        rp.report(node,
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
        rp.report(node,
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
                                       const symbol_info& sym) {
    if (ctx.global_symbol.count(name)) {
        const auto& info = ctx.global_symbol.at(name);
        rp.report(n, "\"" + name +
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
        if (!check_is_specified_enum_member(i.value)) {
            rp.report(i.value, "enum member cannot be specified with float number.");
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
            if (used_generic.count(generic_name)) {
                rp.report(i, "generic type \"" + generic_name +
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
        auto type_name_node = type_node->get_name();
        const auto& basic_type_name = type_name_node->get_name();
        if (!ctx.global_symbol.count(basic_type_name) &&
            !ctx.generics.count(basic_type_name)) {
            rp.report(type_name_node,
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
            rp.report(i, "field name already exists");
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

void regist_pass::regist_global_funcs(ast::root* node) {
    for(auto i : node->get_decls()) {
        if (i->get_ast_type() != ast_type::ast_func_decl) {
            continue;
        }
        auto func_decl = static_cast<ast::func_decl*>(i);
        regist_single_global_func(func_decl);
    }
}

void regist_pass::regist_single_global_func(ast::func_decl* node) {
    const auto& name = node->get_name();
    if (ctx.global_symbol.count(name)) {
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
        std::unordered_set<std::string> used_generic;
        for(auto i : node->get_generic_types()->get_types()) {
            const auto& generic_name = i->get_name()->get_name();
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
    generate_parameter_list(node->get_params(), func_self);
    generate_return_type(node->get_return_type(), func_self);
    if (node->get_name()=="main" && func_self.return_type.is_void()) {
        rp.warning(node, "main function should return integer.");
    }
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

    ctx.generics = {};
    for(const auto& i : stct.generic_template) {
        ctx.generics.insert(i);
    }
    for(auto i : node->get_methods()) {
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

void regist_pass::generate_method_parameter_list(param_list* node,
                                                 colgm_func& self,
                                                 const colgm_struct& stct) {
    for(auto i : node->get_params()) {
        bool is_self = i->get_name()->get_name()=="self";
        if (is_self && i!=node->get_params().front()) {
            rp.report(i->get_name(), "\"self\" must be the first parameter.");
        }
        if (is_self && i->get_type()) {
            rp.warning(i->get_type(), "\"self\" does not need type.");
        }
        if (is_self && !i->get_type()) {
            i->set_type(new type_def(i->get_name()->get_location()));
            i->get_type()->set_name(new identifier(
                i->get_name()->get_location(),
                stct.name
            ));
            i->get_type()->add_pointer_level();
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

    regist_global_funcs(ast_root);
    if (err.geterr()) {
        return;
    }

    regist_impls(ast_root);
    if (err.geterr()) {
        return;
    }

    gnv.visit(ast_root);
    gnv.dump();
}

}