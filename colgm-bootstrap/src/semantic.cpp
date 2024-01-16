#include "semantic.h"

namespace colgm {

void semantic::analyse_single_struct(struct_decl* node) {
    if (ctx.structs.count(node->get_name())) {
        report(node, "struct \"" + node->get_name() + "\" already exists.");
        return;
    }
    ctx.structs.insert({node->get_name(), {}});
    auto& struct_self = ctx.structs.at(node->get_name());
    struct_self.name = node->get_name();
    for(auto i : node->get_fields()) {
        auto type_node = i->get_type();
        auto type_name = type_node->get_name();
        if (!ctx.global_symbol.count(type_name->get_name())) {
            report(type_name, "undefined type \"" + type_name->get_name() + "\".");
        }
        struct_self.field.push_back({
            i->get_name()->get_name(),
            i->get_type()->get_name()->get_name(),
            i->get_type()->get_pointer_level()
        });
    }
}

void semantic::analyse_structs(root* ast_root) {
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_struct_decl) {
            continue;
        }
        auto struct_decl_node = reinterpret_cast<struct_decl*>(i);
        ctx.global_symbol.insert({
            struct_decl_node->get_name(),
            symbol_kind::struct_kind
        });
        analyse_single_struct(struct_decl_node);
    }
}

void semantic::analyse_parameter(param* node, colgm_func& func_self) {
    const auto& name = node->get_name()->get_name();
    if (func_self.find_parameter(name)) {
        report(node->get_name(), "redefinition of parameter \"" + name + "\".");
        return;
    }
    func_self.add_parameter(name, resolve_type_def(node->get_type()));
}

void semantic::analyse_method_parameter_list(param_list* node,
                                             colgm_func& func_self,
                                             const colgm_struct& stct) {
    for(auto i : node->get_params()) {
        if (i->get_name()->get_name()=="self" &&
            i!=node->get_params().front()) {
            report(i->get_name(), "\"self\" must be the first parameter.");
        }
        analyse_parameter(i, func_self);
    }
    if (func_self.parameters.empty() ||
        func_self.parameters.front().name!="self") {
        return;
    }
    const auto& self_type = func_self.parameters.front().symbol_type;
    if (self_type.name!=stct.name || self_type.pointer_level!=1) {
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
    analyse_method_parameter_list(node->get_params(), func_self, stct);
    analyse_return_type(node->get_return_type(), func_self);
    return func_self;
}

colgm_func semantic::analyse_single_func(func_decl* node) {
    auto func_self = colgm_func();
    func_self.name = node->get_name();
    analyse_func_parameter_list(node->get_params(), func_self);
    analyse_return_type(node->get_return_type(), func_self);
    return func_self;
}

void semantic::analyse_functions(root* ast_root) {
    for(auto i : ast_root->get_decls()) {
        if (i->get_ast_type()!=ast_type::ast_func_decl) {
            continue;
        }
        auto func_decl_node = reinterpret_cast<func_decl*>(i);
        ctx.global_symbol.insert({func_decl_node->get_name(), symbol_kind::func_kind});
        if (ctx.functions.count(func_decl_node->get_name())) {
            report(func_decl_node,
                "function \"" + func_decl_node->get_name() + "\" already exists."
            );
            continue;
        }
        ctx.functions.insert({
            func_decl_node->get_name(),
            analyse_single_func(func_decl_node)
        });
    }
}

void semantic::analyse_single_impl(impl_struct* node) {
    if (!ctx.structs.count(node->get_struct_name())) {
        report(node, "undefined struct \"" + node->get_struct_name() + "\".");
        return;
    }
    auto& stct = ctx.structs.at(node->get_struct_name());
    for(auto i : node->get_methods()) {
        if (stct.method.count(i->get_name())) {
            report(i, "method \"" + i->get_name() + "\" already exists.");
            continue;
        }
        stct.method.insert({
            i->get_name(),
            analyse_single_method(i, stct)
        });
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
                                          const std::string& fn_name) {
    auto infer = type({"", 0, true});
    infer.ssm_info = {true, st_name, fn_name};
    return infer;
}

type semantic::resolve_binary_operator(binary_operator* node) {
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());
    if (left!=right) {
        report(node,
            "get \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
    }
    // TODO
    return type::error_type();
}

type semantic::resolve_number_literal(number_literal* node) {
    const auto& literal_string = node->get_number();
    if (literal_string.find(".")!=std::string::npos ||
        literal_string.find("e")!=std::string::npos) {
        node->set_resolve_type({"f64", 0});
        return {"f64", 0};
    }
    f64 result = atof(literal_string.c_str());
    if (std::isinf(result) || std::isnan(result)) {
        report(node, "invalid number \"" + literal_string + "\".");
        return type::error_type();
    }
    if (result <= 2147483647) {
        node->set_resolve_type({"i32", 0});
        return {"i32", 0};
    }
    node->set_resolve_type({"i64", 0});
    return {"i64", 0};
}

type semantic::resolve_string_literal(string_literal* node) {
    ctx.constant_string.insert(node->get_string());
    node->set_resolve_type({"i8", 1});
    return {"i8", 1};
}

type semantic::resolve_bool_literal(bool_literal* node) {
    node->set_resolve_type({"bool", 0});
    return {"bool", 0};
}

type semantic::resolve_identifier(identifier* node) {
    const auto& name = node->get_name();
    if (ctx.find_symbol(name)) {
        return ctx.get_symbol(name);
    }
    if (ctx.global_symbol.count(name)) {
        return {name, 0, true};
    }
    report(node, "undefined symbol \"" + name + "\".");
    return type::error_type();
}

type semantic::resolve_call_field(const type& prev, call_field* node) {
    // TODO
    return type::error_type();
}

type semantic::resolve_call_func_args(const type& prev, call_func_args* node) {
    // TODO
    if (prev.ssm_info.flag_is_ssm) {
        // TODO
        report(node, "call ssm, true.");
    }
    return type::error_type();
}

type semantic::resolve_call_index(const type& prev, call_index* node) {
    if (prev.is_global) {
        report(node,
            "cannot get index from global symbol \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    if (!prev.pointer_level) {
        report(node,
            "cannot get index from \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    auto result = prev;
    result.pointer_level--;
    return result;
}

type semantic::resolve_call_path(const type& prev, call_path* node) {
    if (!prev.is_global) {
        report(node, "cannot get path from a value.");
        return type::error_type();
    }
    if (ctx.structs.count(prev.name) && prev.is_global) {
        const auto& st = ctx.structs.at(prev.name);
        if (st.method.count(node->get_name())) {
            return struct_static_method_infer(prev.name, node->get_name());
        } else {
            report(node,
                "cannot find static method \"" + node->get_name() +
                "\" in \"" + prev.name + "\"."
            );
            return type::error_type();
        }
    }
    return type::error_type();
}

type semantic::resolve_ptr_call_field(const type& prev, ptr_call_field* node) {
    if (prev.is_global) {
        report(node,
            "cannot get field from global symbol \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    if (prev.pointer_level!=1) {
        report(node, "cannot get field from \"" + prev.to_string() + "\".");
        return type::error_type();
    }

    if (!ctx.structs.count(prev.name)) {
        report(node, "cannot get field from \"" + prev.to_string() + "\".");
        return type::error_type();
    }

    const auto& struct_self = ctx.structs.at(prev.name);
    for (const auto& field : struct_self.field) {
        if (node->get_name()==field.name) {
            return field.symbol_type;
        }
    }
    for (const auto& method : struct_self.method) {
        if (node->get_name()==method.first) {
            // TODO
            return type::error_type();
        }
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
        switch(i->get_ast_type()) {
        case ast_type::ast_call_field:
            infer = resolve_call_field(
                infer, reinterpret_cast<call_field*>(i)
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
        case ast_type::ast_ptr_call_field:
            infer = resolve_ptr_call_field(
                infer, reinterpret_cast<ptr_call_field*>(i)
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
    if (ctx.functions.count(infer.name)) {
        report(node, "function should be called here.");
        return type::error_type();
    }
    node->set_resolve_type(infer);
    return infer;
}

type semantic::resolve_assignment(assignment* node) {
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());
    if (left!=right) {
        report(node,
            "get \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
    }
    // TODO
    return type::bool_type();
}

type semantic::resolve_expression(expr* node) {
    switch(node->get_ast_type()) {
    case ast_type::ast_binary_operator:
        return resolve_binary_operator(reinterpret_cast<binary_operator*>(node));
    case ast_type::ast_number_literal:
        return resolve_number_literal(reinterpret_cast<number_literal*>(node));
    case ast_type::ast_string_literal:
        return resolve_string_literal(reinterpret_cast<string_literal*>(node));
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
    return {name, node->get_pointer_level()};
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
    const auto expected_type = resolve_type_def(node->get_type());
    const auto real_type = resolve_expression(node->get_init_value());
    if (expected_type!=real_type) {
        report(node->get_type(),
            "expected \"" + expected_type.to_string() + "\", " +
            "but get \"" + real_type.to_string() + "\"."
        );
    }
    ctx.add_symbol(name, real_type);
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

void semantic::resolve_while_stmt(while_stmt* node, const colgm_func& func_self) {
    const auto infer = resolve_expression(node->get_condition());
    if (infer!=type::bool_type()) {
        report(node->get_condition(),
            "condition should be \"bool\" type but get \"" +
            infer.to_string() + "\"."
        );
    }
    if (node->get_block()) {
        resolve_code_block(node->get_block(), func_self);
    }
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
    if (infer!=func_self.return_type) {
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
    case ast_type::ast_while_stmt:
        resolve_while_stmt(reinterpret_cast<while_stmt*>(node), func_self);
        break;
    case ast_type::ast_in_stmt_expr:
        resolve_in_stmt_expr(reinterpret_cast<in_stmt_expr*>(node), func_self);
        break;
    case ast_type::ast_ret_stmt:
        resolve_ret_stmt(reinterpret_cast<ret_stmt*>(node), func_self);
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
    }
    ctx.pop_new_level();
}

void semantic::resolve_global_func(func_decl* node) {
    if (!node->get_code_block()) {
        return;
    }
    ctx.push_new_level();
    const auto& func_self = ctx.functions.at(node->get_name());
    for(const auto& p : func_self.parameters) {
        ctx.add_symbol(p.name, p.symbol_type);
    }
    for(auto i : node->get_code_block()->get_stmts()) {
        resolve_statement(i, func_self);
    }
    ctx.pop_new_level();
}

void semantic::resolve_method(func_decl* node, const colgm_struct& struct_self) {
    if (!node->get_code_block()) {
        report(node, "should be implemented here.");
        return;
    }
    ctx.push_new_level();
    const auto& method_self = struct_self.method.at(node->get_name());
    for(const auto& p : method_self.parameters) {
        ctx.add_symbol(p.name, p.symbol_type);
    }
    for(auto i : node->get_code_block()->get_stmts()) {
        resolve_statement(i, method_self);
    }
    ctx.pop_new_level();
}

void semantic::resolve_impl(impl_struct* node) {
    const auto& struct_self = ctx.structs.at(node->get_struct_name());
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

const error& semantic::analyse(root* ast_root) {
    ctx.global_symbol.clear();
    ctx.global_symbol.insert({"i64", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"i32", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"i16", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"i8", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"u64", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"u32", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"u16", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"u8", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"f32", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"f64", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"void", symbol_kind::basic_kind});
    ctx.global_symbol.insert({"bool", symbol_kind::basic_kind});

    ctx.structs.clear();
    analyse_structs(ast_root);

    ctx.functions.clear();
    analyse_functions(ast_root);

    analyse_impls(ast_root);
    resolve_function_block(ast_root);
    return err;
}

void semantic::dump() {
    ctx.dump_structs();
    ctx.dump_functions();
}

}