#include "colgm.h"
#include "lexer.h"
#include "parse.h"
#include "sema/semantic.h"
#include "mir/ast2mir.h"

namespace colgm {

void semantic::report_unreachable_statements(code_block* node) {
    bool flag_block_ended = false;
    std::vector<stmt*> unreachable_statements = {};
    for(auto i : node->get_stmts()) {
        if (flag_block_ended) {
            unreachable_statements.push_back(i);
        }
        if (i->get_ast_type()==ast_type::ast_ret_stmt ||
            i->get_ast_type()==ast_type::ast_continue_stmt ||
            i->get_ast_type()==ast_type::ast_break_stmt) {
            flag_block_ended = true;
        }
    }
    if (unreachable_statements.empty()) {
        return;
    }
    auto unreachable_location = unreachable_statements.front()->get_location();
    for(auto i : unreachable_statements) {
        const auto& location = i->get_location();
        unreachable_location.end_column = location.end_column;
        unreachable_location.end_line = location.end_line;
    }
    err.err("sema", unreachable_location, "unreachable statement(s).");
}

void semantic::report_top_level_block_has_no_return(code_block* node) {
    bool flag_has_return = false;
    for(auto i : node->get_stmts()) {
        if (i->get_ast_type()==ast_type::ast_ret_stmt) {
            flag_has_return = true;
            break;
        }
    }
    if (flag_has_return) {
        return;
    }
    report(node, "expect at least one return statement.");
}

colgm_func semantic::builtin_struct_size(const span& loc) {
    auto func = colgm_func();
    func.name = "__size__";
    func.location = loc;
    func.return_type = type::u64_type();
    return func;
}

colgm_func semantic::builtin_struct_alloc(const span& loc, const type& ty) {
    auto func = colgm_func();
    func.name = "__alloc__";
    func.location = loc;
    func.return_type = ty;
    return func;
}

colgm_func semantic::builtin_struct_instance(const span& loc, const type& ty) {
    auto func = colgm_func();
    func.name = "__instance__";
    func.location = loc;
    func.return_type = ty;
    return func;
}

void semantic::analyse_single_struct(struct_decl* node) {
    const auto& name = node->get_name();
    if (ctx.global_symbol.count(name)) {
        report(node, "\"" + name + "\" conflicts with a exist symbol.");
        return;
    }
    if (ctx.global.domain.at(ctx.this_file).structs.count(name)) {
        report(node, "struct \"" + name + "\" conflicts with a exist symbol.");
        return;
    }
    ctx.global.domain.at(ctx.this_file).structs.insert({name, {}});
    ctx.global_symbol.insert({name, {sym_kind::struct_kind, ctx.this_file}});

    // initialize struct info
    auto& this_domain = ctx.global.domain.at(ctx.this_file);
    auto& struct_self = this_domain.structs.at(name);
    struct_self.name = name;
    struct_self.location = node->get_location();

    // load fields
    for(auto i : node->get_fields()) {
        auto type_node = i->get_type();
        auto type_name_node = type_node->get_name();
        if (!ctx.global_symbol.count(type_name_node->get_name())) {
            report(type_name_node,
                "undefined type \"" + type_name_node->get_name() + "\"."
            );
            continue;
        }
        auto field_type = symbol {
            i->get_name()->get_name(), {
                i->get_type()->get_name()->get_name(),
                ctx.global_symbol.at(type_name_node->get_name()).loc_file,
                i->get_type()->get_pointer_level()
            }
        };
        if (struct_self.field.count(i->get_name()->get_name())) {
            report(i, "field name already exists");
        }
        struct_self.field.insert({i->get_name()->get_name(), field_type});
        struct_self.ordered_field.push_back(field_type);
    }

    // add built-in static method size, for malloc usage
    struct_self.static_method.insert(
        {"__size__", builtin_struct_size(node->get_location())}
    );
    struct_self.static_method.insert(
        {"__alloc__", builtin_struct_alloc(node->get_location(), {
            .name = name,
            .loc_file = node->get_location().file,
            .pointer_depth = 1
        })}
    );
    struct_self.static_method.insert(
        {"__instance__", builtin_struct_instance(node->get_location(), {
            .name = name,
            .loc_file = node->get_location().file,
            .pointer_depth = 0
        })}
    );
}

void semantic::analyse_structs(root* ast_root) {
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_struct_decl) {
            continue;
        }
        auto struct_decl_node = reinterpret_cast<struct_decl*>(i);
        analyse_single_struct(struct_decl_node);
    }
}

void semantic::analyse_single_enum(enum_decl* node) {
    const auto& name = node->get_name()->get_name();
    if (ctx.global_symbol.count(name)) {
        report(node, "\"" + name + "\" conflicts with a exist symbol.");
        return;
    }
    if (ctx.global.domain.at(ctx.this_file).structs.count(name)) {
        report(node, "enum \"" + name + "\" conflicts with a exist symbol.");
        return;
    }
    ctx.global.domain.at(ctx.this_file).enums.insert({name, {}});
    ctx.global_symbol.insert({name, {sym_kind::enum_kind, ctx.this_file}});

    // initialize enum info
    auto& this_domain = ctx.global.domain.at(ctx.this_file);
    auto& enum_self = this_domain.enums.at(name);
    enum_self.name = name;
    enum_self.location = node->get_location();

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
        if (!resolve_number_literal(i.value).is_integer()) {
            report(i.value, "enum member cannot be specified with a float number.");
            return;
        }
    }

    for(const auto& i : node->get_member()) {
        if (enum_self.members.count(i.name->get_name())) {
            report(i.name, "enum member already exists");
            continue;
        }
        const auto& member_name = i.name->get_name();
        if (i.value) {
            enum_self.members[member_name] = (usize)str_to_num(i.value->get_number().c_str());
        } else {
            enum_self.members[member_name] = enum_self.ordered_member.size();
        }
        enum_self.ordered_member.push_back(member_name);
    }
}

void semantic::analyse_enums(root* ast_root) {
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_enum_decl) {
            continue;
        }
        auto enum_decl_node = reinterpret_cast<enum_decl*>(i);
        analyse_single_enum(enum_decl_node);
    }
}

void semantic::analyse_parameter(param* node, colgm_func& func_self) {
    const auto& name = node->get_name()->get_name();
    if (func_self.find_parameter(name)) {
        report(node->get_name(),
            "redefinition of parameter \"" + name + "\"."
        );
        return;
    }
    func_self.add_parameter(name, resolve_type_def(node->get_type()));
}

void semantic::analyse_method_parameter_list(param_list* node,
                                             colgm_func& func_self,
                                             const colgm_struct& stct) {
    for(auto i : node->get_params()) {
        bool is_self = i->get_name()->get_name()=="self";
        if (is_self && i!=node->get_params().front()) {
            report(i->get_name(), "\"self\" must be the first parameter.");
        }
        if (is_self && i->get_type()) {
            warning(i->get_type(), "\"self\" does not need type.");
        }
        if (is_self && !i->get_type()) {
            i->set_type(new type_def(i->get_name()->get_location()));
            i->get_type()->set_name(new identifier(
                i->get_name()->get_location(),
                stct.name
            ));
            i->get_type()->add_pointer_level();
        }
        analyse_parameter(i, func_self);
    }

    if (func_self.parameters.empty() ||
        func_self.parameters.front().name!="self") {
        return;
    }

    // we still need to check self type, for user may specify a wrong type
    // check self type is the pointer of implemented struct
    const auto& self_type = func_self.parameters.front().symbol_type;
    if (self_type.name!=stct.name ||
        self_type.loc_file!=stct.location.file ||
        self_type.pointer_depth!=1) {
        report(node->get_params().front(),
            "\"self\" should be \"" + stct.name + "*\", but get \"" +
            self_type.to_string() + "\"."
        );
    }
}

void semantic::analyse_func_parameter_list(param_list* node, colgm_func& func_self) {
    for(auto i : node->get_params()) {
        analyse_parameter(i, func_self);
    }
}

void semantic::analyse_return_type(type_def* node, colgm_func& func_self) {
    func_self.return_type = resolve_type_def(node);
}

colgm_func semantic::analyse_single_method(func_decl* node,
                                           const colgm_struct& stct) {
    auto func_self = colgm_func();
    func_self.name = node->get_name();
    func_self.location = node->get_location();
    analyse_method_parameter_list(node->get_params(), func_self, stct);
    analyse_return_type(node->get_return_type(), func_self);
    return func_self;
}

colgm_func semantic::analyse_single_func(func_decl* node) {
    auto func_self = colgm_func();
    func_self.name = node->get_name();
    func_self.location = node->get_location();
    analyse_func_parameter_list(node->get_params(), func_self);
    analyse_return_type(node->get_return_type(), func_self);
    return func_self;
}

void semantic::analyse_functions(root* ast_root) {
    auto& domain = ctx.global.domain.at(ctx.this_file);
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_func_decl) {
            continue;
        }
        auto func_decl_node = reinterpret_cast<func_decl*>(i);
        if (ctx.global_symbol.count(func_decl_node->get_name())) {
            report(i, "\"" + func_decl_node->get_name() +
                "\" conflicts with a exist symbol."
            );
            continue;
        }
        ctx.global_symbol.insert({
            func_decl_node->get_name(),
            {sym_kind::func_kind, ast_root->get_location().file}
        });
        if (domain.functions.count(func_decl_node->get_name())) {
            report(func_decl_node,
                "function \"" + func_decl_node->get_name() +
                "\" conflicts with a exist symbol."
            );
            continue;
        }
        domain.functions.insert({
            func_decl_node->get_name(),
            analyse_single_func(func_decl_node)
        });
    }
}

void semantic::analyse_single_impl(impl_struct* node) {
    auto& dm = ctx.global.domain.at(ctx.this_file);
    if (!dm.structs.count(node->get_struct_name())) {
        report(node, "undefined struct \"" + node->get_struct_name() + "\".");
        return;
    }
    auto& stct = dm.structs.at(node->get_struct_name());
    for(auto i : node->get_methods()) {
        if (stct.method.count(i->get_name())) {
            report(i, "method \"" + i->get_name() + "\" already exists.");
            continue;
        }
        auto func = analyse_single_method(i, stct);
        if (func.parameters.size() && func.parameters[0].name=="self") {
            stct.method.insert({i->get_name(), func});
        } else {
            stct.static_method.insert({i->get_name(), func});
        }
    }
}

void semantic::analyse_impls(root* ast_root) {
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_impl) {
            continue;
        }
        auto impl_node = reinterpret_cast<impl_struct*>(i);
        analyse_single_impl(impl_node);
    }
}

type semantic::struct_static_method_infer(const std::string& st_name,
                                          const std::string& loc_file,
                                          const std::string& fn_name) {
    auto infer = type({st_name, loc_file, 0, true});
    infer.stm_info = {
        .flag_is_static = true,
        .flag_is_normal = false,
        .method_name = fn_name
    };
    return infer;
}

type semantic::struct_method_infer(const std::string& st_name,
                                   const std::string& loc_file,
                                   const std::string& fn_name) {
    auto infer = type({st_name, loc_file, 0, false});
    infer.stm_info = {
        .flag_is_static = false,
        .flag_is_normal = true,
        .method_name = fn_name
    };
    return infer;
}

type semantic::resolve_logical_operator(binary_operator* node) {
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());
    if (left.is_boolean() && right.is_boolean()) {
        return type::bool_type();
    }
    if (!left.is_boolean()) {
        report(node->get_left(),
            "expect \"bool\" type, but get \"" + left.to_string() + "\"."
        );
    }
    if (!right.is_boolean()) {
        report(node->get_right(),
            "expect \"bool\" type, but get \"" + right.to_string() + "\"."
        );
    }
    node->set_resolve_type(type::bool_type());
    return type::bool_type();
}

type semantic::resolve_comparison_operator(binary_operator* node) {
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());
    if (left!=right) {
        report(node,
            "get \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
        node->set_resolve_type(type::bool_type());
        return type::bool_type();
    }

    // check enum comparison
    if (ctx.search_symbol_kind(left)==sym_kind::enum_kind) {
        if (node->get_opr()!=binary_operator::kind::cmpeq &&
            node->get_opr()!=binary_operator::kind::cmpneq) {
            report(node, "only \"==\" and \"!=\" is allowed.");
        }
        return type::bool_type();
    }

    if (!left.is_integer() && !left.is_float() &&
        !left.is_pointer() && !left.is_boolean()) {
        report(node,
            "cannot compare \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
        node->set_resolve_type(type::bool_type());
        return type::bool_type();
    }
    if (left.is_pointer() && left.pointer_depth!=right.pointer_depth) {
        report(node,
            "cannot compare \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
        node->set_resolve_type(type::bool_type());
        return type::bool_type();
    }
    node->set_resolve_type(type::bool_type());
    return type::bool_type();
}

type semantic::resolve_arithmetic_operator(binary_operator* node) {
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());

    // left hand side value should be the same as right hand side value
    if (left!=right) {
        report(node,
            "get \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
    }

    // cannot calculate enum
    if (ctx.search_symbol_kind(left)==sym_kind::enum_kind) {
        report(node, "cannot calculate enum \"" + left.to_string() + "\".");
        return type::error_type();
    }

    return left;
}

type semantic::resolve_bitwise_operator(binary_operator* node) {
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());
    if (!left.is_integer()) {
        report(node->get_left(),
            "bitwise operator cannot be used on \"" + left.to_string() + "\"."
        );
        return left;
    }
    if (!right.is_integer()) {
        report(node->get_right(),
            "bitwise operator cannot be used on \"" + right.to_string() + "\"."
        );
        return right;
    }
    if (left!=right) {
        report(node,
            "get \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
    }
    return left;
}

type semantic::resolve_binary_operator(binary_operator* node) {
    switch(node->get_opr()) {
        case binary_operator::kind::cmpeq:
        case binary_operator::kind::cmpneq:
        case binary_operator::kind::geq:
        case binary_operator::kind::grt:
        case binary_operator::kind::leq:
        case binary_operator::kind::less: {
            const auto res = resolve_comparison_operator(node);
            node->set_resolve_type(res);
            return res;
        }
        case binary_operator::kind::cmpand:
        case binary_operator::kind::cmpor: {
            const auto res = resolve_logical_operator(node);
            node->set_resolve_type(res);
            return res;
        }
        case binary_operator::kind::add:
        case binary_operator::kind::sub:
        case binary_operator::kind::div:
        case binary_operator::kind::mult:
        case binary_operator::kind::rem: {
            const auto res = resolve_arithmetic_operator(node);
            node->set_resolve_type(res);
            return res;
        }
        case binary_operator::kind::band:
        case binary_operator::kind::bxor:
        case binary_operator::kind::bor: {
            const auto res = resolve_bitwise_operator(node);
            node->set_resolve_type(res);
            return res;
        }
        default: unimplemented(node); break;
    }
    return type::error_type();
}

type semantic::resolve_unary_neg(unary_operator* node) {
    const auto value = resolve_expression(node->get_value());
    if (!value.is_integer() && !value.is_float()) {
        report(node,
            "expect integer or float but get \"" + value.to_string() + "\"."
        );
    }
    return value;
}

type semantic::resolve_unary_bnot(unary_operator* node) {
    const auto value = resolve_expression(node->get_value());
    if (!value.is_integer()) {
        report(node->get_value(),
            "bitwise operator cannot be used on \"" + value.to_string() + "\"."
        );
    }
    return value;
}

type semantic::resolve_unary_lnot(unary_operator* node) {
    const auto value = resolve_expression(node->get_value());
    if (!value.is_boolean()) {
        report(node->get_value(),
            "logical operator cannot be used on \"" + value.to_string() + "\"."
        );
    }
    return type::bool_type();
}

type semantic::resolve_unary_operator(unary_operator* node) {
    switch(node->get_opr()) {
        case unary_operator::kind::neg: {
            const auto res = resolve_unary_neg(node);
            node->set_resolve_type(res);
            return res;
        }
        case unary_operator::kind::bnot: {
            const auto res = resolve_unary_bnot(node);
            node->set_resolve_type(res);
            return res;
        }
        case unary_operator::kind::lnot: {
            const auto res = resolve_unary_lnot(node);
            node->set_resolve_type(res);
            return res;
        }
        default: unimplemented(node); break;
    }
    return type::error_type();
}

type semantic::resolve_type_convert(type_convert* node) {
    const auto res = resolve_expression(node->get_source());
    const auto type_res = resolve_type_def(node->get_target());
    if (res.is_error() || type_res.is_error()) {
        return type::error_type();
    }
    if (res == type_res) {
        report(node->get_target(),
            "unnecessary type cast between \"" +
            res.to_string() + "\" and \"" + type_res.to_string() + "\"."
        );
    }

    // convert floating point number to pointer is unsafe at all
    if ((res.is_float() && type_res.is_pointer()) ||
        (res.is_pointer() && type_res.is_float())) {
        report(node->get_target(),
            "cannot cast floating point number to pointer: \"" +
            res.to_string() + "\" => \"" + type_res.to_string() + "\"."
        );
    }

    // normal struct to pointer is not allowed
    if (ctx.search_symbol_kind(res)==sym_kind::struct_kind &&
        !res.is_pointer() &&
        type_res.is_pointer()) {
        report(node->get_target(),
            "cannot convert \"" + res.to_string() +
            "\" to \"" + type_res.to_string() + "\"."
        );
    }
    // pointer to normal struct is also not allowed
    if (res.is_pointer() &&
        ctx.search_symbol_kind(type_res)==sym_kind::struct_kind &&
        !type_res.is_pointer()) {
        report(node->get_target(),
            "cannot convert \"" + res.to_string() +
            "\" to \"" + type_res.to_string() + "\"."
        );
    }

    node->set_resolve_type(type_res);
    return type_res;
}

type semantic::resolve_nil_literal(nil_literal* node) {
    node->set_resolve_type(type::i8_type(1));
    return type::i8_type(1);
}

type semantic::resolve_number_literal(number_literal* node) {
    const auto& literal_string = node->get_number();
    if (literal_string.find(".")!=std::string::npos ||
        literal_string.find("e")!=std::string::npos) {
        node->set_resolve_type(type::f64_type());
        return type::f64_type();
    }
    f64 result = str_to_num(literal_string.c_str());
    if (std::isinf(result) || std::isnan(result)) {
        report(node, "invalid number \"" + literal_string + "\".");
        return type::error_type();
    }
    if (literal_string.length()>2 && literal_string[1]=='o') {
        node->set_resolve_type(type::u64_type());
        node->reset_number(std::to_string(oct_to_u64(literal_string.c_str())));
        return type::u64_type();
    }
    if (literal_string.length()>2 && literal_string[1]=='x') {
        node->set_resolve_type(type::u64_type());
        node->reset_number(std::to_string(hex_to_u64(literal_string.c_str())));
        return type::u64_type();
    }
    node->set_resolve_type(type::i64_type());
    return type::i64_type();
}

type semantic::resolve_string_literal(string_literal* node) {
    ctx.global.constant_string.insert(node->get_string());
    node->set_resolve_type(type::i8_type(1));
    return type::i8_type(1);
}

type semantic::resolve_char_literal(char_literal* node) {
    node->set_resolve_type(type::i8_type());
    return type::i8_type();
}

type semantic::resolve_bool_literal(bool_literal* node) {
    node->set_resolve_type(type::bool_type());
    return type::bool_type();
}

type semantic::resolve_identifier(identifier* node) {
    const auto& name = node->get_name();
    if (ctx.find_symbol(name)) {
        return ctx.get_symbol(name);
    }
    if (ctx.global_symbol.count(name)) {
        return {
            .name = name,
            .loc_file = ctx.global_symbol.at(name).loc_file,
            .pointer_depth = 0,
            .is_global = true,
            .is_global_func = ctx.global_symbol.at(name).kind==sym_kind::func_kind
        };
    }
    report(node, "undefined symbol \"" + name + "\".");
    return type::error_type();
}

type semantic::resolve_get_field(const type& prev, get_field* node) {
    if (prev.is_global) {
        report(node,
            "cannot get field from global symbol \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    if (prev.is_pointer()) {
        report(node,
            "cannot use \".\" to get field from pointer \"" +
            prev.to_string() + "\". maybe you mean \"->\"?"
        );
        return type::error_type();
    }

    // prev resolved type is natvie type
    if (prev.loc_file.empty()) {
        report(node,
            "cannot get method \"" + node->get_name() +
            "\" from \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }

    const auto& domain = ctx.global.domain.at(prev.loc_file);
    if (!domain.structs.count(prev.name)) {
        report(node, "cannot get field from \"" + prev.to_string() + "\".");
        return type::error_type();
    }

    const auto& struct_self = domain.structs.at(prev.name);
    if (struct_self.field.count(node->get_name())) {
        node->set_resolve_type(struct_self.field.at(node->get_name()).symbol_type);
        return struct_self.field.at(node->get_name()).symbol_type;
    }
    if (struct_self.method.count(node->get_name())) {
        const auto res = struct_method_infer(
            prev.name,
            prev.loc_file,
            node->get_name()
        );
        node->set_resolve_type(res);
        return res;
    }
    if (struct_self.static_method.count(node->get_name())) {
        report(node,
            "method \"" + node->get_name() +
            "\" in \"" + prev.name + "\" is static."
        );
        return type::error_type();
    }
    report(node,
        "cannot find field \"" + node->get_name() +
        "\" in \"" + prev.name + "\"."
    );
    return type::error_type();
}

void semantic::check_static_call_args(const colgm_func& func,
                                      call_func_args* node) {
    if (func.parameters.size()!=node->get_args().size()) {
        report(node,
            "expect " + std::to_string(func.parameters.size()) +
            " argument(s) but get " + std::to_string(node->get_args().size()) +
            "."
        );
        return;
    }
    size_t index = 0;
    for(auto i : node->get_args()) {
        const auto infer = resolve_expression(i);
        const auto param = func.parameters[index].symbol_type;
        if (infer!=param) {
            report(i,
                "expect \"" + param.to_string() +
                "\" but get \"" + infer.to_string() + "\"."
            );
        }
        ++index;
    }
}

void semantic::check_method_call_args(const colgm_func& func,
                                      const type& self,
                                      call_func_args* node) {
    if (func.parameters.size()!=node->get_args().size()+1) {
        report(node,
            "expect " + std::to_string(func.parameters.size()-1) +
            " argument(s) but get " + std::to_string(node->get_args().size()) +
            "."
        );
        return;
    }

    // make sure self is correct, should be unreachable
    if (!self.eq_no_ptr(func.parameters[0].symbol_type)) {
        report(node,
            "self should be \"" + func.parameters[0].symbol_type.to_string() +
            "\" but get \"" + self.to_string() + "\"."
        );
    }

    // check args
    size_t index = 1;
    for(auto i : node->get_args()) {
        const auto infer = resolve_expression(i);
        const auto param = func.parameters[index].symbol_type;
        if (infer!=param) {
            report(i,
                "expect \"" + param.to_string() +
                "\" but get \"" + infer.to_string() + "\"."
            );
        }
        ++index;
    }
}

type semantic::resolve_call_func_args(const type& prev, call_func_args* node) {
    // global function call
    if (prev.is_global_func) {
        const auto& domain = ctx.global.domain.at(prev.loc_file);
        const auto& func = domain.functions.at(prev.name);
        check_static_call_args(func, node);
        node->set_resolve_type(func.return_type);
        return func.return_type;
    }
    // static method call of struct
    if (prev.stm_info.flag_is_static) {
        const auto& domain = ctx.global.domain.at(prev.loc_file);
        const auto& st = domain.structs.at(prev.name);
        const auto& method = st.static_method.at(prev.stm_info.method_name);
        check_static_call_args(method, node);
        node->set_resolve_type(method.return_type);
        return method.return_type;
    }
    // method call of struct
    if (prev.stm_info.flag_is_normal) {
        const auto& domain = ctx.global.domain.at(prev.loc_file);
        const auto& st = domain.structs.at(prev.name);
        const auto& method = st.method.at(prev.stm_info.method_name);
        check_method_call_args(method, prev, node);
        node->set_resolve_type(method.return_type);
        return method.return_type;
    }

    unimplemented(node);
    return type::error_type();
}

type semantic::resolve_call_index(const type& prev, call_index* node) {
    if (prev.is_global) {
        report(node,
            "cannot get index from global symbol \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    if (!prev.is_pointer()) {
        report(node,
            "cannot get index from \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    resolve_expression(node->get_index());
    auto result = prev;
    result.pointer_depth--;
    node->set_resolve_type(result);
    return result;
}

type semantic::resolve_call_path(const type& prev, call_path* node) {
    if (!prev.is_global) {
        report(node, "cannot get path from a value.");
        return type::error_type();
    }

    // prev resolved type is a native type
    if (prev.loc_file.empty()) {
        report(node,
            "cannot get static method \"" + node->get_name() +
            "\" from \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }

    const auto& domain = ctx.global.domain.at(prev.loc_file);
    if (domain.structs.count(prev.name) && prev.is_global) {
        const auto& st = domain.structs.at(prev.name);
        if (st.static_method.count(node->get_name())) {
            const auto res = struct_static_method_infer(
                prev.name,
                prev.loc_file,
                node->get_name()
            );
            node->set_resolve_type(res);
            return res;
        } else {
            report(node,
                "cannot find static method \"" + node->get_name() +
                "\" in \"" + prev.name + "\"."
            );
            return type::error_type();
        }
    }
    if (domain.enums.count(prev.name) && prev.is_global) {
        const auto& en = domain.enums.at(prev.name);
        if (en.members.count(node->get_name())) {
            auto res = prev;
            res.is_global = false;
            node->set_resolve_type(res);
            return res;
        }
    }
    return type::error_type();
}

type semantic::resolve_ptr_get_field(const type& prev, ptr_get_field* node) {
    if (prev.is_global) {
        report(node,
            "cannot get field from global symbol \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    if (prev.pointer_depth!=1) {
        report(node,
            "cannot use \"->\" to get field from \"" +
            prev.to_string() + "\"."
        );
        return type::error_type();
    }

    // prev resolved type is natvie type
    if (prev.loc_file.empty()) {
        report(node,
            "cannot get method \"" + node->get_name() +
            "\" from \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }

    const auto& domain = ctx.global.domain.at(prev.loc_file);
    if (!domain.structs.count(prev.name)) {
        report(node, "cannot get field from \"" + prev.to_string() + "\".");
        return type::error_type();
    }

    const auto& struct_self = domain.structs.at(prev.name);
    if (struct_self.field.count(node->get_name())) {
        node->set_resolve_type(struct_self.field.at(node->get_name()).symbol_type);
        return struct_self.field.at(node->get_name()).symbol_type;
    }
    if (struct_self.method.count(node->get_name())) {
        auto infer = type({prev.name, prev.loc_file, prev.pointer_depth, false});
        infer.stm_info = {
            .flag_is_static = false,
            .flag_is_normal = true,
            .method_name = node->get_name()
        };
        node->set_resolve_type(infer);
        return infer;
    }
    if (struct_self.static_method.count(node->get_name())) {
        report(node,
            "method \"" + node->get_name() +
            "\" in \"" + prev.name + "\" is static."
        );
        return type::error_type();
    }
    report(node,
        "cannot find field \"" + node->get_name() +
        "\" in \"" + prev.name + "\"."
    );
    return type::error_type();
}

type semantic::resolve_call(call* node) {
    auto infer = resolve_identifier(node->get_head());
    node->get_head()->set_resolve_type(infer);
    if (infer.is_error()) {
        return infer;
    }

    // resolve call chain
    for(auto i : node->get_chain()) {
        if (i->get_ast_type() != ast_type::ast_call_func_args &&
            infer.is_function()) {
            report(i, "function should be called before.");
            return type::error_type();
        }
        switch(i->get_ast_type()) {
        case ast_type::ast_get_field:
            infer = resolve_get_field(
                infer, reinterpret_cast<get_field*>(i)
            );
            break;
        case ast_type::ast_call_func_args:
            infer = resolve_call_func_args(
                infer, reinterpret_cast<call_func_args*>(i)
            );
            break;
        case ast_type::ast_call_index:
            infer = resolve_call_index(
                infer, reinterpret_cast<call_index*>(i)
            );
            break;
        case ast_type::ast_call_path:
            infer = resolve_call_path(
                infer, reinterpret_cast<call_path*>(i)
            );
            break;
        case ast_type::ast_ptr_get_field:
            infer = resolve_ptr_get_field(
                infer, reinterpret_cast<ptr_get_field*>(i)
            );
            break;
        default:
            unimplemented(i);
            return type::error_type();
        }
        if (infer.is_error()) {
            return infer;
        }
    }
    if (ctx.global.domain.at(ctx.this_file).functions.count(infer.name)) {
        report(node, "function should be called here.");
        return type::error_type();
    }
    node->set_resolve_type(infer);
    return infer;
}

bool semantic::check_valid_left_value(expr* node) {
    if (node->get_ast_type()!=ast_type::ast_call) {
        return false;
    }
    const auto mem_get_node = reinterpret_cast<call*>(node);
    if (mem_get_node->get_head()->get_ast_type()!=ast_type::ast_identifier) {
        return false;
    }
    for(auto i : mem_get_node->get_chain()) {
        if (i->get_ast_type()==ast_type::ast_call_path ||
            i->get_ast_type()==ast_type::ast_call_func_args) {
            return false;
        }
    }
    return true;
}

type semantic::resolve_assignment(assignment* node) {
    if (!check_valid_left_value(node->get_left())) {
        report(node->get_left(), "bad left value.");
        return type::error_type();
    }
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());
    switch(node->get_type()) {
        case assignment::kind::andeq:
        case assignment::kind::xoreq:
        case assignment::kind::oreq:
            if (!left.is_integer()) {
                report(node->get_left(),
                    "bitwise operator cannot be used on \"" +
                    left.to_string() + "\"."
                );
                return type::bool_type();
            }
            if (!right.is_integer()) {
                report(node->get_right(),
                    "bitwise operator cannot be used on \"" +
                    right.to_string() + "\"."
                );
                return type::bool_type();
            }
        default: break;
    }
    if (left.is_pointer() && right.is_pointer()) {
        if (left!=right) {
            report(node,
                "cannot calculate \"" + left.to_string() +
                "\" and \"" + right.to_string() + "\"."
            );
        }
    } else if (left!=right) {
        report(node,
            "get \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
    }

    // only = is allowed to be applied on enums
    if (ctx.search_symbol_kind(left)==sym_kind::enum_kind &&
        node->get_type()!=assignment::kind::eq) {
        report(node, "cannot calculate enum \"" + left.to_string() + "\".");
        return type::bool_type();
    }
    return type::bool_type();
}

type semantic::resolve_expression(expr* node) {
    switch(node->get_ast_type()) {
    case ast_type::ast_unary_operator:
        return resolve_unary_operator(reinterpret_cast<unary_operator*>(node));
    case ast_type::ast_binary_operator:
        return resolve_binary_operator(reinterpret_cast<binary_operator*>(node));
    case ast_type::ast_type_convert:
        return resolve_type_convert(reinterpret_cast<type_convert*>(node));
    case ast_type::ast_nil_literal:
        return resolve_nil_literal(reinterpret_cast<nil_literal*>(node));
    case ast_type::ast_number_literal:
        return resolve_number_literal(reinterpret_cast<number_literal*>(node));
    case ast_type::ast_string_literal:
        return resolve_string_literal(reinterpret_cast<string_literal*>(node));
    case ast_type::ast_char_literal:
        return resolve_char_literal(reinterpret_cast<char_literal*>(node));
    case ast_type::ast_bool_literal:
        return resolve_bool_literal(reinterpret_cast<bool_literal*>(node));
    case ast_type::ast_call:
        return resolve_call(reinterpret_cast<call*>(node));
    case ast_type::ast_assignment:
        return resolve_assignment(reinterpret_cast<assignment*>(node));
    default:
        unimplemented(node);
        return type::error_type();
    }
    unreachable(node);
    return type::error_type();
}

type semantic::resolve_type_def(type_def* node) {
    const auto& name = node->get_name()->get_name();
    if (!ctx.global_symbol.count(name)) {
        report(node->get_name(), "unknown type \"" + name + "\".");
        return type::error_type();
    }
    return {name, ctx.global_symbol.at(name).loc_file, node->get_pointer_level()};
}

void semantic::resolve_definition(definition* node, const colgm_func& func_self) {
    const auto& name = node->get_name();
    if (ctx.find_symbol(name)) {
        report(node, "redefinition of variable \"" + name + "\".");
        return;
    }
    if (ctx.global_symbol.count(name)) {
        report(node, "variable \"" + name + "\" conflicts with global symbol.");
        return;
    }
    // must have init value!
    if (!node->get_init_value()) {
        report(node, "expect initial value.");
        return;
    }
    // no type declaration
    if (!node->get_type()) {
        const auto real_type = resolve_expression(node->get_init_value());
        node->set_resolve_type(real_type);
        ctx.add_symbol(name, real_type);
        return;
    }
    const auto expected_type = resolve_type_def(node->get_type());
    const auto real_type = resolve_expression(node->get_init_value());
    if (expected_type.is_pointer() && real_type.is_pointer()) {
        if (expected_type!=real_type) {
            warning(node,
                "expected \"" + expected_type.to_string() +
                "\", but get \"" + real_type.to_string() + "\"."
            );
        }
    } else if (expected_type!=real_type) {
        report(node->get_type(),
            "expected \"" + expected_type.to_string() +
            "\", but get \"" + real_type.to_string() + "\"."
        );
    }
    node->set_resolve_type(expected_type);
    ctx.add_symbol(name, expected_type);
}

void semantic::resolve_if_stmt(if_stmt* node, const colgm_func& func_self) {
    if (node->get_condition()) {
        const auto infer = resolve_expression(node->get_condition());
        if (infer!=type::bool_type()) {
            report(node->get_condition(),
                "condition should be \"bool\" type but get \"" +
                infer.to_string() + "\"."
            );
        }
    }
    if (node->get_block()) {
        resolve_code_block(node->get_block(), func_self);
    }
}

void semantic::resolve_cond_stmt(cond_stmt* node, const colgm_func& func_self) {
    for(auto i : node->get_stmts()) {
        resolve_if_stmt(i, func_self);
    }
}

bool semantic::check_is_enum_literal(expr* node) {
    if (node->get_ast_type()!=ast_type::ast_call) {
        return false;
    }
    auto call_node = reinterpret_cast<call*>(node);
    if (call_node->get_chain().size()!=1) {
        return false;
    }

    const auto& name = call_node->get_head()->get_name();
    if (!ctx.global_symbol.count(name)) {
        return false;
    }
    if (ctx.global_symbol.at(name).kind!=sym_kind::enum_kind) {
        return false;
    }

    if (call_node->get_chain()[0]->get_ast_type()!=ast_type::ast_call_path) {
        return false;
    }
    return true;
}

void semantic::resolve_match_stmt(match_stmt* node, const colgm_func& func_self) {
    const auto infer = resolve_expression(node->get_value());
    node->get_value()->set_resolve_type(infer);
    if (ctx.search_symbol_kind(infer)!=sym_kind::enum_kind) {
        report(node->get_value(), "match value should be enum type.");
        return;
    }
    if (node->get_cases().empty()) {
        report(node, "expect at least one case.");
        return;
    }
    for(auto i : node->get_cases()) {
        const auto case_node = i->get_value();
        const auto case_infer = resolve_expression(case_node);
        case_node->set_resolve_type(case_infer);
        if (case_infer!=infer) {
            report(case_node,
                "case value should be \"" + infer.to_string() +
                "\" type but get \"" + case_infer.to_string() + "\"."
            );
            continue;
        }

        if (!check_is_enum_literal(case_node)) {
            report(case_node, "case value should be enum literal.");
            continue;
        }

        resolve_code_block(i->get_block(), func_self);
    }
}

void semantic::resolve_while_stmt(while_stmt* node, const colgm_func& func_self) {
    const auto infer = resolve_expression(node->get_condition());
    if (infer!=type::bool_type()) {
        report(node->get_condition(),
            "condition should be \"bool\" type but get \"" +
            infer.to_string() + "\"."
        );
    }
    if (node->get_block()) {
        ++in_loop_level;
        resolve_code_block(node->get_block(), func_self);
        --in_loop_level;
    }
}

void semantic::resolve_for_stmt(for_stmt* node, const colgm_func& func_self) {
    ctx.push_new_level();
    if (node->get_init()) {
        resolve_definition(node->get_init(), func_self);
    }
    if (node->get_condition()) {
        const auto infer = resolve_expression(node->get_condition());
        node->get_condition()->set_resolve_type(infer);
        if (infer!=type::bool_type()) {
            report(node->get_condition(),
                "condition should be \"bool\" type but get \"" +
                infer.to_string() + "\"."
            );
        }
    }
    if (node->get_update()) {
        const auto infer = resolve_expression(node->get_update());
        node->get_update()->set_resolve_type(infer);
    }
    if (node->get_block()) {
        ++in_loop_level;
        resolve_code_block(node->get_block(), func_self);
        --in_loop_level;
    }
    ctx.pop_new_level();
}

void semantic::resolve_in_stmt_expr(in_stmt_expr* node, const colgm_func& func_self) {
    node->set_resolve_type(resolve_expression(node->get_expr()));
}

void semantic::resolve_ret_stmt(ret_stmt* node, const colgm_func& func_self) {
    if (!node->get_value() && func_self.return_type!=type::void_type()) {
        report(node,
            "expected return type \"" + func_self.return_type.to_string() +
            "\" but get \"void\"."
        );
        return;
    }
    if (!node->get_value()) {
        return;
    }
    const auto infer = resolve_expression(node->get_value());
    if (infer.is_pointer() && func_self.return_type.is_pointer()) {
        if (infer!=func_self.return_type) {
            warning(node,
                "expected return type \"" + func_self.return_type.to_string() +
                "\" but get \"" + infer.to_string() + "\"."
            );
        }
    } else if (infer!=func_self.return_type) {
        report(node,
            "expected return type \"" + func_self.return_type.to_string() +
            "\" but get \"" + infer.to_string() + "\"."
        );
    }
}

void semantic::resolve_statement(stmt* node, const colgm_func& func_self) {
    switch(node->get_ast_type()) {
    case ast_type::ast_definition:
        resolve_definition(reinterpret_cast<definition*>(node), func_self);
        break;
    case ast_type::ast_cond_stmt:
        resolve_cond_stmt(reinterpret_cast<cond_stmt*>(node), func_self);
        break;
    case ast_type::ast_match_stmt:
        resolve_match_stmt(reinterpret_cast<match_stmt*>(node), func_self);
        break;
    case ast_type::ast_while_stmt:
        resolve_while_stmt(reinterpret_cast<while_stmt*>(node), func_self);
        break;
    case ast_type::ast_for_stmt:
        resolve_for_stmt(reinterpret_cast<for_stmt*>(node), func_self);
        break;
    case ast_type::ast_in_stmt_expr:
        resolve_in_stmt_expr(reinterpret_cast<in_stmt_expr*>(node), func_self);
        break;
    case ast_type::ast_ret_stmt:
        resolve_ret_stmt(reinterpret_cast<ret_stmt*>(node), func_self);
        break;
    case ast_type::ast_continue_stmt:
    case ast_type::ast_break_stmt:
        if (!in_loop_level) {
            report(node, "this statement should be used inside a loop.");
        }
        break;
    default:
        unreachable(node);
        break;
    }
}

void semantic::resolve_code_block(code_block* node, const colgm_func& func_self) {
    // should not be called to resolve function's top code block
    ctx.push_new_level();
    for(auto i : node->get_stmts()) {
        resolve_statement(i, func_self);
        if (i->get_ast_type()==ast_type::ast_ret_stmt ||
            i->get_ast_type()==ast_type::ast_continue_stmt ||
            i->get_ast_type()==ast_type::ast_break_stmt) {
            report_unreachable_statements(node);
            break;
        }
    }
    ctx.pop_new_level();
}

void semantic::resolve_global_func(func_decl* node) {
    if (!node->get_code_block()) {
        return;
    }
    const auto& domain = ctx.global.domain.at(ctx.this_file);
    if (!domain.functions.count(node->get_name())) {
        report(node, "cannot find function \"" + node->get_name() + "\".");
        return;
    }
    ctx.push_new_level();
    const auto& func_self = domain.functions.at(node->get_name());
    for(const auto& p : func_self.parameters) {
        ctx.add_symbol(p.name, p.symbol_type);
    }
    for(auto i : node->get_code_block()->get_stmts()) {
        resolve_statement(i, func_self);
        if (i->get_ast_type()==ast_type::ast_ret_stmt ||
            i->get_ast_type()==ast_type::ast_continue_stmt ||
            i->get_ast_type()==ast_type::ast_break_stmt) {
            report_unreachable_statements(node->get_code_block());
            break;
        }
    }
    report_top_level_block_has_no_return(node->get_code_block());
    ctx.pop_new_level();
}

void semantic::resolve_method(func_decl* node, const colgm_struct& struct_self) {
    if (!node->get_code_block()) {
        report(node, "should be implemented here.");
        return;
    }
    ctx.push_new_level();
    const auto& method_self =
        struct_self.method.count(node->get_name())?
        struct_self.method.at(node->get_name()):
        struct_self.static_method.at(node->get_name());
    for(const auto& p : method_self.parameters) {
        ctx.add_symbol(p.name, p.symbol_type);
    }
    for(auto i : node->get_code_block()->get_stmts()) {
        resolve_statement(i, method_self);
        if (i->get_ast_type()==ast_type::ast_ret_stmt ||
            i->get_ast_type()==ast_type::ast_continue_stmt ||
            i->get_ast_type()==ast_type::ast_break_stmt) {
            report_unreachable_statements(node->get_code_block());
            break;
        }
    }
    report_top_level_block_has_no_return(node->get_code_block());
    ctx.pop_new_level();
}

void semantic::resolve_impl(impl_struct* node) {
    const auto& domain = ctx.global.domain.at(ctx.this_file);
    if (!domain.structs.count(node->get_struct_name())) {
        report(node,
            "cannot implement \"" + node->get_struct_name() +
            "\", this struct is not defined in this file."
        );
        return;
    }
    const auto& struct_self = domain.structs.at(node->get_struct_name());
    for(auto i : node->get_methods()) {
        resolve_method(i, struct_self);
    }
}

void semantic::resolve_function_block(root* ast_root) {
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()==ast_type::ast_impl) {
            resolve_impl(reinterpret_cast<impl_struct*>(i));
        }
        if (i->get_ast_type()==ast_type::ast_func_decl) {
            resolve_global_func(reinterpret_cast<func_decl*>(i));
        }
    }
}

void semantic::import_global_symbol(node* n, 
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

void semantic::resolve_single_use(use_stmt* node) {
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
            "\" is not totally analysed, maybe encounter self-reference."
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
            return;
        }
        if (par.analyse(lex.result()).geterr()) {
            return;
        }
        if (sema.analyse(par.get_result()).geterr()) {
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

    const auto& domain = ctx.global.domain.at(file);
    if (node->get_import_symbol().empty()) {
        for(const auto& i : domain.structs) {
            import_global_symbol(node, i.second.name, {sym_kind::struct_kind, file});
        }
        for(const auto& i : domain.functions) {
            import_global_symbol(node, i.second.name, {sym_kind::func_kind, file});
        }
        return;
    }
    // specified import
    for(auto i : node->get_import_symbol()) {
        if (domain.structs.count(i->get_name())) {
            import_global_symbol(i, i->get_name(), {sym_kind::struct_kind, file});
        }
        if (domain.functions.count(i->get_name())) {
            import_global_symbol(i, i->get_name(), {sym_kind::func_kind, file});
        }
    }
}

void semantic::resolve_use_stmt(root* node) {
    for(auto i : node->get_use_stmts()) {
        resolve_single_use(i);
    }
}

const error& semantic::analyse(root* ast_root) {
    ctx.this_file = ast_root->get_location().file;
    if (!ctx.global.domain.count(ctx.this_file)) {
        ctx.global.domain.insert({ctx.this_file, {}});
    }

    ctx.global_symbol.clear();
    ctx.global_symbol.insert({"i64", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"i32", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"i16", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"i8", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"u64", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"u32", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"u16", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"u8", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"f32", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"f64", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"void", {sym_kind::basic_kind, ""}});
    ctx.global_symbol.insert({"bool", {sym_kind::basic_kind, ""}});
    resolve_use_stmt(ast_root);
    if (err.geterr()) {
        return err;
    }

    // load enum first
    ctx.global.domain.at(ctx.this_file).enums.clear();
    analyse_enums(ast_root);

    ctx.global.domain.at(ctx.this_file).structs.clear();
    analyse_structs(ast_root);

    ctx.global.domain.at(ctx.this_file).functions.clear();
    analyse_functions(ast_root);

    analyse_impls(ast_root);

    // resolve pass
    resolve_function_block(ast_root);
    return err;
}

void semantic::dump() {
    ctx.dump_enums();
    ctx.dump_structs();
    ctx.dump_functions();
}

}