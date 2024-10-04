#include "colgm.h"
#include "lexer.h"
#include "parse.h"
#include "sema/semantic.h"
#include "sema/regist.h"
#include "mir/ast2mir.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

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
    rp.report(unreachable_location, "unreachable statement(s).");
}

void semantic::report_top_level_block_has_no_return(code_block* node,
                                                    const colgm_func& func) {
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

    // if return type is void and without return statement, just add one
    if (func.return_type.is_void()) {
        node->add_stmt(new ret_stmt(node->get_location()));
        return;
    }

    // report
    rp.report(node, "expect at least one return statement.");
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
        rp.report(node->get_left(),
            "expect \"bool\" type, but get \"" + left.to_string() + "\"."
        );
    }
    if (!right.is_boolean()) {
        rp.report(node->get_right(),
            "expect \"bool\" type, but get \"" + right.to_string() + "\"."
        );
    }
    node->set_resolve_type(type::bool_type());
    return type::bool_type();
}

type semantic::resolve_comparison_operator(binary_operator* node) {
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());
    if (left.is_error() || right.is_error()) {
        return type::bool_type();
    }
    if (left!=right) {
        rp.report(node,
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
            rp.report(node, "only \"==\" and \"!=\" is allowed.");
        }
        return type::bool_type();
    }

    if (!left.is_integer() && !left.is_float() &&
        !left.is_pointer() && !left.is_boolean()) {
        rp.report(node,
            "cannot compare \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
        node->set_resolve_type(type::bool_type());
        return type::bool_type();
    }
    if (left.is_pointer() && left.pointer_depth!=right.pointer_depth) {
        rp.report(node,
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
        rp.report(node,
            "get \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
    }

    // cannot calculate enum
    if (ctx.search_symbol_kind(left)==sym_kind::enum_kind) {
        rp.report(node, "cannot calculate enum \"" + left.to_string() + "\".");
        return type::error_type();
    }

    return left;
}

type semantic::resolve_bitwise_operator(binary_operator* node) {
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());
    if (!left.is_integer()) {
        rp.report(node->get_left(),
            "bitwise operator cannot be used on \"" + left.to_string() + "\"."
        );
        return left;
    }
    if (!right.is_integer()) {
        rp.report(node->get_right(),
            "bitwise operator cannot be used on \"" + right.to_string() + "\"."
        );
        return right;
    }
    if (left!=right) {
        rp.report(node,
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
        default: rp.unimplemented(node); break;
    }
    return type::error_type();
}

type semantic::resolve_unary_neg(unary_operator* node) {
    const auto value = resolve_expression(node->get_value());
    if (!value.is_integer() && !value.is_float()) {
        rp.report(node,
            "expect integer or float but get \"" + value.to_string() + "\"."
        );
    }
    return value;
}

type semantic::resolve_unary_bnot(unary_operator* node) {
    const auto value = resolve_expression(node->get_value());
    if (!value.is_integer()) {
        rp.report(node->get_value(),
            "bitwise operator cannot be used on \"" + value.to_string() + "\"."
        );
    }
    return value;
}

type semantic::resolve_unary_lnot(unary_operator* node) {
    const auto value = resolve_expression(node->get_value());
    if (!value.is_boolean()) {
        rp.report(node->get_value(),
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
        default: rp.unimplemented(node); break;
    }
    return type::error_type();
}

type semantic::resolve_type_convert(type_convert* node) {
    const auto res = resolve_expression(node->get_source());
    const auto type_res = rs.resolve_type_def(node->get_target());
    if (res.is_error() || type_res.is_error()) {
        return type::error_type();
    }
    if (res == type_res) {
        rp.report(node->get_target(),
            "unnecessary type cast between \"" +
            res.to_string() + "\" and \"" + type_res.to_string() + "\"."
        );
    }

    // convert floating point number to pointer is unsafe at all
    if ((res.is_float() && type_res.is_pointer()) ||
        (res.is_pointer() && type_res.is_float())) {
        rp.report(node->get_target(),
            "cannot cast floating point number to pointer: \"" +
            res.to_string() + "\" => \"" + type_res.to_string() + "\"."
        );
    }

    // normal struct to pointer is not allowed
    if (ctx.search_symbol_kind(res)==sym_kind::struct_kind &&
        !res.is_pointer() &&
        type_res.is_pointer()) {
        rp.report(node->get_target(),
            "cannot convert \"" + res.to_string() +
            "\" to \"" + type_res.to_string() + "\"."
        );
    }
    // pointer to normal struct is also not allowed
    if (res.is_pointer() &&
        ctx.search_symbol_kind(type_res)==sym_kind::struct_kind &&
        !type_res.is_pointer()) {
        rp.report(node->get_target(),
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
        rp.report(node, "invalid number \"" + literal_string + "\".");
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

type semantic::resolve_array_literal(array_literal* node) {
    const auto size_infer = resolve_number_literal(node->get_size());
    if (!size_infer.is_integer()) {
        rp.report(node->get_size(), "array size must be integer.");
    }
    const auto type_infer = rs.resolve_type_def(node->get_type());
    node->set_resolve_type(type_infer.get_pointer_copy());
    auto result_type = type_infer.get_pointer_copy();
    result_type.is_immutable_array_address = true;
    return result_type;
}

type semantic::resolve_identifier(identifier* node) {
    const auto& name = node->get_name();
    if (ctx.find_symbol(name)) {
        return ctx.get_symbol(name);
    }
    if (ctx.global_symbol.count(name)) {
        const auto& sym = ctx.global_symbol.at(name);
        if (!sym.is_public) {
            rp.report(node, "\"" + name + "\" is not imported.");
            return type::error_type();
        }
        return {
            .name = name,
            .loc_file = sym.loc_file,
            .pointer_depth = 0,
            .is_global = true,
            .is_global_func = sym.kind==sym_kind::func_kind
        };
    }
    rp.report(node, "undefined symbol \"" + name + "\".");
    return type::error_type();
}

void semantic::check_pub_method(ast::node* node,
                                const std::string& name,
                                const colgm_struct& self) {
    if (!self.method.count(name)) {
        return;
    }
    if (self.method.at(name).is_public) {
        return;
    }
    if (impl_struct_name.empty() || impl_struct_name!=self.name) {
        rp.report(node,
            "cannot call private method \"" + self.name + "::" + name + "\"."
        );
    }
}

void semantic::check_pub_static_method(ast::node* node,
                                       const std::string& name,
                                       const colgm_struct& self) {
    if (!self.static_method.count(name)) {
        return;
    }
    if (self.static_method.at(name).is_public) {
        return;
    }
    if (impl_struct_name.empty()) {
        rp.report(node,
            "cannot call private static method \"" +
            self.name + "::" + name + "\"."
        );
    }
}

type semantic::resolve_get_field(const type& prev, get_field* node) {
    if (prev.is_global) {
        rp.report(node,
            "cannot get field from global symbol \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    if (prev.is_pointer()) {
        rp.report(node,
            "cannot use \".\" to get field from pointer \"" +
            prev.to_string() + "\". maybe you mean \"->\"?"
        );
        return type::error_type();
    }

    // prev resolved type is natvie type
    if (prev.loc_file.empty()) {
        rp.report(node,
            "cannot get method \"" + node->get_name() +
            "\" from \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }

    const auto& domain = ctx.global.domain.at(prev.loc_file);
    if (!domain.structs.count(prev.name)) {
        rp.report(node, "cannot get field from \"" + prev.to_string() + "\".");
        return type::error_type();
    }

    const auto& struct_self = domain.structs.at(prev.name);
    if (struct_self.field.count(node->get_name())) {
        node->set_resolve_type(struct_self.field.at(node->get_name()).symbol_type);
        return struct_self.field.at(node->get_name()).symbol_type;
    }
    if (struct_self.method.count(node->get_name())) {
        check_pub_method(node, node->get_name(), struct_self);
        const auto res = struct_method_infer(
            prev.name,
            prev.loc_file,
            node->get_name()
        );
        node->set_resolve_type(res);
        return res;
    }
    if (struct_self.static_method.count(node->get_name())) {
        rp.report(node,
            "method \"" + node->get_name() +
            "\" in \"" + prev.name + "\" is static."
        );
        return type::error_type();
    }
    rp.report(node,
        "cannot find field \"" + node->get_name() +
        "\" in \"" + prev.name + "\"."
    );
    return type::error_type();
}

void semantic::check_static_call_args(const colgm_func& func,
                                      call_func_args* node) {
    if (func.parameters.size()!=node->get_args().size()) {
        rp.report(node,
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
        // do not report if infer is error, because it must be reported before
        if (infer!=param && !infer.is_error()) {
            rp.report(i,
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
        rp.report(node,
            "expect " + std::to_string(func.parameters.size()-1) +
            " argument(s) but get " + std::to_string(node->get_args().size()) +
            "."
        );
        return;
    }

    // make sure self is correct, should be unreachable
    if (!self.eq_no_ptr(func.parameters[0].symbol_type)) {
        rp.report(node,
            "self should be \"" + func.parameters[0].symbol_type.to_string() +
            "\" but get \"" + self.to_string() + "\"."
        );
    }

    // check args
    size_t index = 1;
    for(auto i : node->get_args()) {
        const auto infer = resolve_expression(i);
        const auto param = func.parameters[index].symbol_type;
        // do not report if infer is error, because it must be reported before
        if (infer!=param && !infer.is_error()) {
            rp.report(i,
                "expect \"" + param.to_string() +
                "\" but get \"" + infer.to_string() + "\"."
            );
        }
        ++index;
    }
}

type semantic::resolve_call_id(call_id* node) {
    auto infer = resolve_identifier(node->get_id());
    node->set_resolve_type(infer);
    if (infer.is_error()) {
        return infer;
    }
    if (node->get_generic_types() && !infer.is_global) {
        rp.report(node,
            "non global symbol \"" + node->get_id()->get_name() +
            "\" cannot be generic."
        );
    }
    return infer;
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

    rp.report(node, "cannot call non-function.");
    return type::error_type();
}

type semantic::resolve_call_index(const type& prev, call_index* node) {
    if (prev.is_global) {
        rp.report(node,
            "cannot get index from global symbol \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    if (!prev.is_pointer()) {
        rp.report(node,
            "cannot get index from \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    resolve_expression(node->get_index());
    auto result = prev;
    result.pointer_depth--;
    if (result.is_immutable_array_address) {
        result.is_immutable_array_address = false;
    }
    node->set_resolve_type(result);
    return result;
}

type semantic::resolve_initializer(const type& prev, initializer* node) {
    if (!prev.is_global) {
        rp.report(node, "need a global symbol to initialize.");
        return type::error_type();
    }

    if (prev.loc_file.empty()) {
        rp.report(node, "basic type cannot be initialized as a struct.");
        return type::error_type();
    }

    const auto& domain = ctx.global.domain.at(prev.loc_file);
    if (!domain.structs.count(prev.name)) {
        rp.report(node, "\"" + prev.name + "\" is not a struct type, cannot initialize.");
        return type::error_type();
    }

    const auto& st = domain.structs.at(prev.name);
    for(auto i : node->get_pairs()) {
        const auto& field = i->get_field()->get_name();
        if (!st.field.count(field)) {
            rp.report(i,
                "cannot find field \"" + field + "\" in \"" + prev.name + "\"."
            );
            continue;
        }

        const auto infer = resolve_expression(i->get_value());
        i->get_value()->set_resolve_type(infer);

        if (infer != st.field.at(field).symbol_type) {
            rp.report(i,
                "expect \"" + st.field.at(field).symbol_type.to_string() +
                "\" but get \"" + infer.to_string() + "\"."
            );
            continue;
        }
    }

    auto copy = prev;
    copy.is_global = false;
    node->set_resolve_type(copy);
    return copy;
}

type semantic::resolve_call_path(const type& prev, call_path* node) {
    if (!prev.is_global) {
        rp.report(node, "cannot get path from a value.");
        return type::error_type();
    }

    // prev resolved type is a native type
    if (prev.loc_file.empty()) {
        rp.report(node,
            "cannot get static method \"" + node->get_name() +
            "\" from \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }

    const auto& domain = ctx.global.domain.at(prev.loc_file);
    if (domain.structs.count(prev.name) && prev.is_global) {
        const auto& st = domain.structs.at(prev.name);
        if (st.static_method.count(node->get_name())) {
            check_pub_static_method(node, node->get_name(), st);
            const auto res = struct_static_method_infer(
                prev.name,
                prev.loc_file,
                node->get_name()
            );
            node->set_resolve_type(res);
            return res;
        } else {
            rp.report(node,
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
        rp.report(node,
            "cannot find enum member \"" + node->get_name() +
            "\" in \"" + prev.name + "\"."
        );
    }
    return type::error_type();
}

type semantic::resolve_ptr_get_field(const type& prev, ptr_get_field* node) {
    if (prev.is_global) {
        rp.report(node,
            "cannot get field from global symbol \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }
    if (prev.pointer_depth!=1) {
        rp.report(node,
            "cannot use \"->\" to get field from \"" +
            prev.to_string() + "\"."
        );
        return type::error_type();
    }

    // prev resolved type is natvie type
    if (prev.loc_file.empty()) {
        rp.report(node,
            "cannot get method \"" + node->get_name() +
            "\" from \"" + prev.to_string() + "\"."
        );
        return type::error_type();
    }

    const auto& domain = ctx.global.domain.at(prev.loc_file);
    if (!domain.structs.count(prev.name)) {
        rp.report(node, "cannot get field from \"" + prev.to_string() + "\".");
        return type::error_type();
    }

    const auto& struct_self = domain.structs.at(prev.name);
    if (struct_self.field.count(node->get_name())) {
        node->set_resolve_type(struct_self.field.at(node->get_name()).symbol_type);
        return struct_self.field.at(node->get_name()).symbol_type;
    }
    if (struct_self.method.count(node->get_name())) {
        check_pub_method(node, node->get_name(), struct_self);
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
        rp.report(node,
            "method \"" + node->get_name() +
            "\" in \"" + prev.name + "\" is static."
        );
        return type::error_type();
    }
    rp.report(node,
        "cannot find field \"" + node->get_name() +
        "\" in \"" + prev.name + "\"."
    );
    return type::error_type();
}

type semantic::resolve_call(call* node) {
    auto infer = resolve_call_id(node->get_head());
    if (infer.is_error()) {
        return infer;
    }

    // resolve call chain
    for(auto i : node->get_chain()) {
        if (i->get_ast_type() != ast_type::ast_call_func_args &&
            infer.is_function()) {
            rp.report(i, "function should be called before.");
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
        case ast_type::ast_initializer:
            infer = resolve_initializer(
                infer, reinterpret_cast<initializer*>(i)
            );
            break;
        case ast_type::ast_ptr_get_field:
            infer = resolve_ptr_get_field(
                infer, reinterpret_cast<ptr_get_field*>(i)
            );
            break;
        default:
            rp.unimplemented(i);
            return type::error_type();
        }
        if (infer.is_error()) {
            return infer;
        }
    }
    if (infer.stm_info.flag_is_normal || infer.stm_info.flag_is_static) {
        rp.report(node, "function should be called here.");
        return type::error_type();
    }
    if (ctx.global.domain.at(ctx.this_file).functions.count(infer.name)) {
        rp.report(node, "function should be called here.");
        return type::error_type();
    }
    node->set_resolve_type(infer);
    if (infer.is_global) {
        rp.report(node,
            "get global \"" + infer.to_string() + "\" type, " +
            "but not an instance."
        );
    }
    return infer;
}

bool semantic::check_valid_left_value(expr* node) {
    if (node->get_ast_type()!=ast_type::ast_call) {
        return false;
    }
    const auto mem_get_node = reinterpret_cast<call*>(node);
    for(auto i : mem_get_node->get_chain()) {
        if (i->get_ast_type()==ast_type::ast_call_path ||
            i->get_ast_type()==ast_type::ast_call_func_args ||
            i->get_ast_type()==ast_type::ast_initializer) {
            return false;
        }
    }
    return true;
}

void semantic::check_mutable_left_value(expr* node, const type& lt) {
    if (lt.is_immutable_array_address) {
        rp.report(node, "cannot assign to immutable array address.");
        return;
    }
    if (lt.is_constant_type) {
        rp.report(node, "cannot assign to \"const " + lt.to_string() + "\".");
        return;
    }
    return;
}

type semantic::resolve_assignment(assignment* node) {
    if (!check_valid_left_value(node->get_left())) {
        rp.report(node->get_left(), "bad left value.");
        return type::error_type();
    }
    const auto left = resolve_expression(node->get_left());
    check_mutable_left_value(node->get_left(), left);
    const auto right = resolve_expression(node->get_right());
    if (left.is_error() || right.is_error()) {
        return type::error_type();
    }
    switch(node->get_type()) {
        case assignment::kind::andeq:
        case assignment::kind::xoreq:
        case assignment::kind::oreq:
            if (!left.is_integer()) {
                rp.report(node->get_left(),
                    "bitwise operator cannot be used on \"" +
                    left.to_string() + "\"."
                );
                return type::bool_type();
            }
            if (!right.is_integer()) {
                rp.report(node->get_right(),
                    "bitwise operator cannot be used on \"" +
                    right.to_string() + "\"."
                );
                return type::bool_type();
            }
        default: break;
    }
    if (left.is_pointer() && right.is_pointer()) {
        if (left!=right) {
            rp.report(node,
                "cannot calculate \"" + left.to_string() +
                "\" and \"" + right.to_string() + "\"."
            );
        }
    } else if (left!=right) {
        rp.report(node,
            "get \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\"."
        );
    }

    // only = is allowed to be applied on enums
    if (ctx.search_symbol_kind(left)==sym_kind::enum_kind &&
        node->get_type()!=assignment::kind::eq) {
        rp.report(node, "cannot calculate enum \"" + left.to_string() + "\".");
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
    case ast_type::ast_array_literal:
        return resolve_array_literal(reinterpret_cast<array_literal*>(node));
    case ast_type::ast_call:
        return resolve_call(reinterpret_cast<call*>(node));
    case ast_type::ast_assignment:
        return resolve_assignment(reinterpret_cast<assignment*>(node));
    default:
        rp.unimplemented(node);
        return type::error_type();
    }
    rp.unreachable(node);
    return type::error_type();
}

void semantic::resolve_definition(definition* node, const colgm_func& func_self) {
    const auto& name = node->get_name();
    if (ctx.find_symbol(name)) {
        rp.report(node, "redefinition of variable \"" + name + "\".");
        return;
    }
    if (ctx.global_symbol.count(name)) {
        rp.report(node, "variable \"" + name + "\" conflicts with global symbol.");
        return;
    }
    // must have init value!
    if (!node->get_init_value()) {
        rp.report(node, "expect initial value.");
        return;
    }
    // no type declaration
    if (!node->get_type()) {
        const auto real_type = resolve_expression(node->get_init_value());
        node->set_resolve_type(real_type);
        ctx.add_symbol(name, real_type);
        check_defined_variable_is_void(node, real_type);
        return;
    }

    // with type declaration
    const auto expected_type = rs.resolve_type_def(node->get_type());
    const auto real_type = resolve_expression(node->get_init_value());
    if (expected_type.is_pointer() && real_type.is_pointer()) {
        if (expected_type!=real_type) {
            rp.warning(node,
                "expected \"" + expected_type.to_string() +
                "\", but get \"" + real_type.to_string() + "\"."
            );
        }
    } else if (expected_type!=real_type) {
        rp.report(node->get_type(),
            "expected \"" + expected_type.to_string() +
            "\", but get \"" + real_type.to_string() + "\"."
        );
    }

    // if immutable, make sure the type is correct
    if (real_type.is_constant_type || real_type.is_immutable_array_address) {
        node->set_resolve_type(real_type);
        ctx.add_symbol(name, real_type);
        check_defined_variable_is_void(node, real_type);
    } else {
        node->set_resolve_type(expected_type);
        ctx.add_symbol(name, expected_type);
        check_defined_variable_is_void(node, expected_type);
    }
}

void semantic::check_defined_variable_is_void(definition* node, const type& t) {
    if (!t.is_void()) {
        return;
    }
    rp.report(node, "cannot define variable with void type.");
}

void semantic::resolve_if_stmt(if_stmt* node, const colgm_func& func_self) {
    if (node->get_condition()) {
        const auto infer = resolve_expression(node->get_condition());
        if (infer!=type::bool_type() && infer!=type::error_type()) {
            rp.report(node->get_condition(),
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

    const auto& name = call_node->get_head()->get_id()->get_name();
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
        rp.report(node->get_value(), "match value should be enum type.");
        return;
    }
    if (node->get_cases().empty()) {
        rp.report(node, "expect at least one case.");
        return;
    }
    for(auto i : node->get_cases()) {
        const auto case_node = i->get_value();
        const auto case_infer = resolve_expression(case_node);
        case_node->set_resolve_type(case_infer);
        if (case_infer!=infer) {
            rp.report(case_node,
                "case value should be \"" + infer.to_string() +
                "\" type but get \"" + case_infer.to_string() + "\"."
            );
            continue;
        }

        if (!check_is_enum_literal(case_node)) {
            rp.report(case_node, "case value should be enum literal.");
            continue;
        }

        resolve_code_block(i->get_block(), func_self);
    }
}

void semantic::resolve_while_stmt(while_stmt* node, const colgm_func& func_self) {
    const auto infer = resolve_expression(node->get_condition());
    if (infer!=type::bool_type() && infer!=type::error_type()) {
        rp.report(node->get_condition(),
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
        if (infer!=type::bool_type() && infer!=type::error_type()) {
            rp.report(node->get_condition(),
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
        rp.report(node,
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
        if (infer!=func_self.return_type && infer!=type::error_type()) {
            rp.warning(node,
                "expected return type \"" + func_self.return_type.to_string() +
                "\" but get \"" + infer.to_string() + "\"."
            );
        }
    } else if (infer!=func_self.return_type && infer!=type::error_type()) {
        rp.report(node,
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
            rp.report(node, "this statement should be used inside a loop.");
        }
        break;
    default:
        rp.unreachable(node);
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
    if (node->get_generic_types()) {
        return; // do not resolve generic impl
    }
    const auto& domain = ctx.global.domain.at(ctx.this_file);
    if (!domain.functions.count(node->get_name())) {
        rp.report(node, "cannot find function \"" + node->get_name() + "\".");
        return;
    }
    const auto& func_self = domain.functions.at(node->get_name());
    if (!node->get_code_block()) {
        if (!func_self.is_extern) {
            rp.warning(node,
                "non-extern function \"" + node->get_name() +
                "\" is not implemented."
            );
        }
        return;
    }
    ctx.push_new_level();
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
    report_top_level_block_has_no_return(node->get_code_block(), func_self);
    ctx.pop_new_level();
}

void semantic::resolve_method(func_decl* node, const colgm_struct& struct_self) {
    if (!node->get_code_block()) {
        rp.report(node, "should be implemented here.");
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
    report_top_level_block_has_no_return(node->get_code_block(), method_self);
    ctx.pop_new_level();
}

void semantic::resolve_impl(impl_struct* node) {
    if (node->get_generic_types()) {
        return; // do not resolve generic impl
    }
    const auto& domain = ctx.global.domain.at(ctx.this_file);
    if (!domain.structs.count(node->get_struct_name())) {
        rp.report(node,
            "cannot implement \"" + node->get_struct_name() +
            "\", this struct is not defined in this file."
        );
        return;
    }
    const auto& struct_self = domain.structs.at(node->get_struct_name());
    impl_struct_name = node->get_struct_name();
    for(auto i : node->get_methods()) {
        resolve_method(i, struct_self);
    }
    impl_struct_name = "";
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
    ctx.this_file = ast_root->get_file();
    if (!ctx.global.domain.count(ctx.this_file)) {
        ctx.global.domain.insert({ctx.this_file, {}});
    }

    regist_pass(err, ctx).run(ast_root);
    if (err.geterr()) {
        return err;
    }

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