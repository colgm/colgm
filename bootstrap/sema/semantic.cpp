#include "colgm.h"
#include "lexer.h"
#include "parse/parse.h"
#include "sema/semantic.h"
#include "sema/regist_pass.h"
#include "mir/ast2mir.h"

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

namespace colgm {

void semantic::report_unreachable_statements(code_block* node) {
    bool flag_block_ended = false;
    std::vector<stmt*> unreachable_statements = {};
    for (auto i : node->get_stmts()) {
        if (flag_block_ended) {
            unreachable_statements.push_back(i);
        }
        if (i->is(ast_type::ast_ret_stmt) ||
            i->is(ast_type::ast_continue_stmt) ||
            i->is(ast_type::ast_break_stmt)) {
            flag_block_ended = true;
        }
    }
    if (unreachable_statements.empty()) {
        return;
    }
    auto unreachable_location = unreachable_statements.front()->get_location();
    for (auto i : unreachable_statements) {
        const auto& location = i->get_location();
        unreachable_location.end_column = location.end_column;
        unreachable_location.end_line = location.end_line;
    }
    rp.warn(
        unreachable_location,
        unreachable_statements.size() > 1
        ? "unreachable statements, ignored"
        : "unreachable statement, ignored"
    );
}

void semantic::report_top_level_block_has_no_return(code_block* node,
                                                    const colgm_func& func) {
    bool flag_has_return = false;
    for (auto i : node->get_stmts()) {
        if (i->is(ast_type::ast_ret_stmt)) {
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
    rp.report(node, "expect at least one return statement");
}

bool semantic::number_literal_can_be_converted(node* n,
                                               const type& expect) {
    // if is not number literal, just return false
    if (!n->is(ast_type::ast_number_literal)) {
        return false;
    }

    // check if number literal is pointer type
    // in most cases it should not be pointer type
    const auto& n_type = n->get_resolve();
    if (n_type.is_pointer()) {
        return false;
    }

    // if expect a pointer type,
    if (expect.is_pointer()) {
        // allow integer to convert to pointer may cause confusion
        // because if user used C before, in this example:
        //   <i64*> s += 1
        // in fact this is equal to:
        //   <i8*> s += 8
        // to avoid this confusion, we should not allow this conversion
        return false;
    }

    // use same integer type as expected
    if (expect.is_integer() && n_type.is_integer()) {
        n->set_resolve_type(expect);
        return true;
    }
    // use same float type as expected
    if (expect.is_float() && n_type.is_float()) {
        n->set_resolve_type(expect);
        return true;
    }
    return false;
}

bool semantic::unary_number_can_be_converted(node* n,
                                             const type& expect) {
    if (!n->is(ast_type::ast_unary_operator)) {
        return false;
    }
    auto UO = static_cast<unary_operator*>(n);
    if (UO->get_opr() != unary_operator::kind::neg) {
        return false;
    }
    if (number_literal_can_be_converted(UO->get_value(), expect)) {
        n->set_resolve_type(expect);
        return true;
    }
    return false;
}

bool semantic::nil_can_be_converted(node* n, const type& expect) {
    if (!n->is(ast_type::ast_nil_literal)) {
        return false;
    }
    if (!expect.is_pointer()) {
        return false;
    }
    n->set_resolve_type(expect);
    return true;
}

bool semantic::check_can_be_converted(node* n, const type& expect) {
    if (number_literal_can_be_converted(n, expect)) {
        return true;
    }
    if (unary_number_can_be_converted(n, expect)) {
        return true;
    }
    if (nil_can_be_converted(n, expect)) {
        return true;
    }
    return false;
}

type semantic::struct_static_method_infer(const type& prev,
                                          const std::string& fn_name) {
    auto infer = prev;
    infer.pointer_depth = 0;
    infer.is_global = true;
    infer.stm_info = {
        .flag_is_static = true,
        .flag_is_normal = false,
        .method_name = fn_name
    };
    return infer;
}

type semantic::struct_method_infer(const type& prev,
                                   const std::string& fn_name) {
    auto infer = prev;
    infer.pointer_depth = 0;
    infer.is_global = true;
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
    if (left.is_error() || right.is_error()) {
        node->set_resolve_type(type::bool_type());
        return type::bool_type();
    }
    if (left.is_boolean() && right.is_boolean()) {
        node->set_resolve_type(type::bool_type());
        return type::bool_type();
    }
    if (!left.is_boolean()) {
        rp.report(node->get_left(),
            "expect \"bool\" type, but get \"" + left.to_string() + "\""
        );
    }
    if (!right.is_boolean()) {
        rp.report(node->get_right(),
            "expect \"bool\" type, but get \"" + right.to_string() + "\""
        );
    }
    node->set_resolve_type(type::bool_type());
    return type::bool_type();
}

type semantic::resolve_comparison_operator(binary_operator* node) {
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());
    if (left.is_error() || right.is_error()) {
        node->set_resolve_type(type::bool_type());
        return type::bool_type();
    }
    if (left != right) {
        if (!check_can_be_converted(node->get_right(), left) &&
            !check_can_be_converted(node->get_left(), right)) {
            rp.report(node,
                "get \"" + left.to_string() +
                "\" and \"" + right.to_string() + "\""
            );
        }
        node->set_resolve_type(type::bool_type());
        return type::bool_type();
    }

    // check enum comparison
    if (ctx.search_symbol_kind(left)==sym_kind::enum_kind) {
        if (node->get_opr()!=binary_operator::kind::cmpeq &&
            node->get_opr()!=binary_operator::kind::cmpneq) {
            rp.report(node, "only \"==\" and \"!=\" is allowed");
        }
        node->set_resolve_type(type::bool_type());
        return type::bool_type();
    }

    if (!left.is_integer() && !left.is_float() &&
        !left.is_pointer() && !left.is_boolean()) {
        rp.report(node,
            "cannot compare \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\""
        );
        node->set_resolve_type(type::bool_type());
        return type::bool_type();
    }
    if (left.is_pointer() && left.pointer_depth != right.pointer_depth) {
        rp.report(node,
            "cannot compare \"" + left.to_string() +
            "\" and \"" + right.to_string() + "\""
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
    if (left != right) {
        if (!check_can_be_converted(node->get_right(), left) &&
            !check_can_be_converted(node->get_left(), right)) {
            rp.report(node,
                "get \"" + left.to_string() +
                "\" and \"" + right.to_string() + "\""
            );
        }
    }

    // cannot calculate enum
    if (ctx.search_symbol_kind(left)==sym_kind::enum_kind) {
        rp.report(node->get_left(), "cannot calculate enum \"" + left.to_string() + "\"");
        return type::error_type();
    } else if (ctx.search_symbol_kind(right)==sym_kind::enum_kind) {
        rp.report(node->get_right(), "cannot calculate enum \"" + right.to_string() + "\"");
        return type::error_type();
    }

    node->set_resolve_type(left);
    return left;
}

type semantic::resolve_bitwise_operator(binary_operator* node) {
    const auto left = resolve_expression(node->get_left());
    const auto right = resolve_expression(node->get_right());
    if (!left.can_bitwise_calculate()) {
        rp.report(node->get_left(),
            "bitwise operator cannot be used on \"" + left.to_string() + "\""
        );
        node->set_resolve_type(left);
        return left;
    }
    if (!right.can_bitwise_calculate()) {
        rp.report(node->get_right(),
            "bitwise operator cannot be used on \"" + right.to_string() + "\""
        );
        node->set_resolve_type(right);
        return right;
    }
    if (left != right) {
        if (!check_can_be_converted(node->get_right(), left) &&
            !check_can_be_converted(node->get_left(), right)) {
            rp.report(node,
                "get \"" + left.to_string() +
                "\" and \"" + right.to_string() + "\""
            );
        }
    }
    node->set_resolve_type(left);
    return left;
}

type semantic::resolve_binary_operator(binary_operator* node) {
    switch(node->get_opr()) {
        case binary_operator::kind::cmpeq:
        case binary_operator::kind::cmpneq:
        case binary_operator::kind::geq:
        case binary_operator::kind::grt:
        case binary_operator::kind::leq:
        case binary_operator::kind::less:
            return resolve_comparison_operator(node);
        case binary_operator::kind::cmpand:
        case binary_operator::kind::cmpor:
            return resolve_logical_operator(node);
        case binary_operator::kind::add:
        case binary_operator::kind::sub:
        case binary_operator::kind::div:
        case binary_operator::kind::mult:
        case binary_operator::kind::rem:
            return resolve_arithmetic_operator(node);
        case binary_operator::kind::band:
        case binary_operator::kind::bxor:
        case binary_operator::kind::bor:
            return resolve_bitwise_operator(node);
        default: rp.unimplemented(node); break;
    }
    return type::error_type();
}

type semantic::resolve_unary_neg(unary_operator* node) {
    const auto value = resolve_expression(node->get_value());
    if (value.is_error()) {
        return value;
    }
    if (!value.is_integer() && !value.is_float()) {
        rp.report(node,
            "expect integer or float but get \"" + value.to_string() + "\""
        );
    }
    return value;
}

type semantic::resolve_unary_bnot(unary_operator* node) {
    const auto value = resolve_expression(node->get_value());
    if (value.is_error()) {
        return value;
    }
    if (!value.can_bitwise_calculate()) {
        rp.report(node->get_value(),
            "bitwise operator cannot be used on \"" + value.to_string() + "\""
        );
    }
    return value;
}

type semantic::resolve_unary_lnot(unary_operator* node) {
    const auto value = resolve_expression(node->get_value());
    // do not report if get error type, because the related error is reported before
    if (value.is_error()) {
        return type::bool_type();
    }
    if (!value.is_boolean() || value.is_pointer()) {
        rp.report(node->get_value(),
            "logical operator cannot be used on \"" + value.to_string() + "\""
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
    const auto type_res = tr.resolve(node->get_target());
    if (res.is_error() || type_res.is_error()) {
        return type::error_type();
    }

    // convert floating point number to pointer is unsafe at all
    if ((res.is_float() && !res.is_pointer() && type_res.is_pointer()) ||
        (res.is_pointer() && type_res.is_float() && !type_res.is_pointer())) {
        rp.report(node->get_target(),
            "cannot cast floating point number to pointer: \"" +
            res.to_string() + "\" => \"" + type_res.to_string() + "\""
        );
    }

    // normal struct to any type is not allowed
    if (ctx.search_symbol_kind(res)==sym_kind::struct_kind &&
        !res.is_pointer()) {
        rp.report(node->get_target(),
            "cannot convert \"" + res.to_string() +
            "\" to \"" + type_res.to_string() + "\""
        );
    }
    // any type to normal struct is also not allowed
    if (ctx.search_symbol_kind(type_res)==sym_kind::struct_kind &&
        !type_res.is_pointer()) {
        rp.report(node->get_target(),
            "cannot convert \"" + res.to_string() +
            "\" to \"" + type_res.to_string() + "\""
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
    f64 result = str_to_num(literal_string.c_str());
    if (std::isinf(result) || std::isnan(result)) {
        rp.report(node, "invalid number \"" + literal_string + "\"");
        return type::error_type();
    }
    if (literal_string.find(".") != std::string::npos ||
        literal_string.find("e") != std::string::npos) {
        node->set_resolve_type(type::f64_type());
        return type::f64_type();
    }
    if (literal_string.length() > 2 && literal_string[1] == 'o') {
        node->set_resolve_type(type::u64_type());
        node->reset_number(std::to_string(oct_to_u64(literal_string.c_str())));
        return type::u64_type();
    }
    if (literal_string.length() > 2 && literal_string[1] == 'x') {
        node->set_resolve_type(type::u64_type());
        node->reset_number(std::to_string(hex_to_u64(literal_string.c_str())));
        return type::u64_type();
    }
    node->set_resolve_type(type::i64_type());
    return type::i64_type();
}

type semantic::resolve_string_literal(string_literal* node) {
    ctx.global.constant_string.insert(node->get_string());
    node->set_resolve_type(type::const_str_literal_type());
    return type::const_str_literal_type();
}

type semantic::resolve_char_literal(char_literal* node) {
    node->set_resolve_type(type::i8_type());
    return type::i8_type();
}

type semantic::resolve_bool_literal(bool_literal* node) {
    node->set_resolve_type(type::bool_type());
    return type::bool_type();
}

type semantic::resolve_array_list(array_list* node) {
    std::vector<type> list_type;
    for (auto i : node->get_value()) {
        list_type.push_back(resolve_expression(i));
    }
    for (const auto& t : list_type) {
        if (t.is_error()) {
            return type::error_type();
        }
    }
    if (list_type.empty()) {
        return type::empty_array_type();
    }

    const auto& marked_base_type = list_type[0];
    for (int i = 1; i < list_type.size(); i++) {
        if (list_type[i] != marked_base_type &&
            !check_can_be_converted(node->get_value()[i], marked_base_type)) {
            rp.report(node->get_value()[i],
                "expect \"" +  marked_base_type.to_string() +
                "\" but get \"" + list_type[i].to_string() + "\""
            );
        }
    }

    auto result_type = marked_base_type.get_pointer_copy();
    result_type.is_array = true;
    result_type.array_length = list_type.size();
    node->set_resolve_type(result_type);
    return result_type;
}

type semantic::resolve_identifier(identifier* node) {
    const auto& name = node->get_name();
    if (ctx.find_local(name)) {
        return ctx.get_local(name);
    }

    const auto& dm = node->is_redirected()
        ? ctx.get_domain(node->get_redirect_location())
        : ctx.get_domain(node->get_file());
    if (dm.global_symbol.count(name)) {
        const auto& sym = dm.global_symbol.at(name);
        if (!sym.is_public) {
            rp.report(node, "\"" + name + "\" is not imported");
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
    rp.report(node, "undefined symbol \"" + name + "\"");
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
            "cannot call private method \"" + self.name + "::" + name + "\""
        );
    }
}

void semantic::check_pub_method(ast::node* node,
                                const std::string& name,
                                const colgm_tagged_union& self) {
    if (!self.method.count(name)) {
        return;
    }
    if (self.method.at(name).is_public) {
        return;
    }
    if (impl_struct_name.empty() || impl_struct_name!=self.name) {
        rp.report(node,
            "cannot call private method \"" + self.name + "::" + name + "\""
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
            self.name + "::" + name + "\""
        );
    }
}

void semantic::check_pub_static_method(ast::node* node,
                                       const std::string& name,
                                       const colgm_tagged_union& self) {
    if (!self.static_method.count(name)) {
        return;
    }
    if (self.static_method.at(name).is_public) {
        return;
    }
    if (impl_struct_name.empty()) {
        rp.report(node,
            "cannot call private static method \"" +
            self.name + "::" + name + "\""
        );
    }
}

type semantic::resolve_get_field(const type& prev, get_field* node) {
    if (prev.is_global) {
        rp.report(node,
            "cannot get field from global symbol \"" + prev.to_string() + "\""
        );
        return type::error_type();
    }
    if (prev.is_pointer()) {
        rp.report(node->get_operator_location(),
            "cannot use \".\" to get field from pointer \"" +
            prev.to_string() + "\".",
            "maybe you mean \"->\"?"
        );
        return type::error_type();
    }

    // prev resolved type is natvie type
    if (prev.loc_file.empty()) {
        rp.report(node,
            "cannot get method \"" + node->get_name() +
            "\" from \"" + prev.to_string() + "\""
        );
        return type::error_type();
    }

    const auto& domain = ctx.get_domain(prev.loc_file);
    if (domain.structs.count(prev.name_for_search())) {
        return resolve_struct_get_field(
            domain.structs.at(prev.name_for_search()),
            prev,
            node
        );
    } else if (domain.tagged_unions.count(prev.name_for_search())) {
        return resolve_tagged_union_get_field(
            domain.tagged_unions.at(prev.name_for_search()),
            prev,
            node
        );
    }

    rp.report(node,
        "cannot get field from \"" +
        prev.full_path_name() +
        "\""
    );
    return type::error_type();
}

type semantic::resolve_struct_get_field(const colgm_struct& struct_self,
                                        const type& prev,
                                        get_field* node) {
    if (struct_self.field.count(node->get_name())) {
        node->set_resolve_type(struct_self.field.at(node->get_name()));
        return struct_self.field.at(node->get_name());
    }
    if (struct_self.method.count(node->get_name())) {
        check_pub_method(node, node->get_name(), struct_self);
        const auto res = struct_method_infer(prev, node->get_name());
        node->set_resolve_type(res);
        return res;
    }
    if (struct_self.static_method.count(node->get_name())) {
        rp.report(node,
            "method \"" + node->get_name() +
            "\" in \"" + prev.name_for_search() + "\" is static"
        );
        return type::error_type();
    }
    rp.report(node,
        "cannot find field \"" + node->get_name() +
        "\" in \"" + prev.name_for_search() + "\"",
        "maybe you mean \"" + struct_self.fuzzy_match_field(node->get_name()) + "\"?"
    );
    return type::error_type();
}

type semantic::resolve_tagged_union_get_field(const colgm_tagged_union& un,
                                              const type& prev,
                                              get_field* node) {
    if (un.member.count(node->get_name())) {
        node->set_resolve_type(un.member.at(node->get_name()));
        return un.member.at(node->get_name());
    }
    if (un.method.count(node->get_name())) {
        check_pub_method(node, node->get_name(), un);
        const auto res = struct_method_infer(prev, node->get_name());
        node->set_resolve_type(res);
        return res;
    }
    if (un.static_method.count(node->get_name())) {
        rp.report(node,
            "method \"" + node->get_name() +
            "\" in \"" + prev.name_for_search() + "\" is static"
        );
        return type::error_type();
    }
    rp.report(node,
        "cannot find field \"" + node->get_name() +
        "\" in \"" + prev.name_for_search() + "\"",
        "maybe you mean \"" + un.fuzzy_match_field(node->get_name()) + "\"?"
    );
    return type::error_type();
}

void semantic::check_static_call_args(const colgm_func& func,
                                      call_func_args* node) {
    if (func.ordered_params.size() != node->get_args().size()) {
        rp.report(node,
            "expect " + std::to_string(func.ordered_params.size()) +
            " argument(s) but get " + std::to_string(node->get_args().size())
        );
        return;
    }
    size_t index = 0;
    for (auto i : node->get_args()) {
        const auto infer = resolve_expression(i);
        const auto& param = func.params.at(func.ordered_params[index]);
        // do not report if infer is error, because it must be reported before
        if (infer != param && !infer.is_error()) {
            if (!check_can_be_converted(i, param)) {
                rp.report(i,
                    "expect \"" + param.to_string() +
                    "\" but get \"" + infer.to_string() + "\""
                );
            }
        }
        ++index;
    }
}

void semantic::check_method_call_args(const colgm_func& func,
                                      call_func_args* node) {
    if (func.ordered_params.size() != node->get_args().size() + 1) {
        rp.report(node,
            "expect " + std::to_string(func.ordered_params.size() - 1) +
            " argument(s) but get " + std::to_string(node->get_args().size())
        );
        return;
    }

    // check args
    size_t index = 1;
    for (auto i : node->get_args()) {
        const auto infer = resolve_expression(i);
        const auto& param = func.params.at(func.ordered_params[index]);
        // do not report if infer is error, because it must be reported before
        if (infer != param && !infer.is_error()) {
            if (!check_can_be_converted(i, param)) {
                rp.report(i,
                    "expect \"" + param.to_string() +
                    "\" but get \"" + infer.to_string() + "\""
                );
            }
        }
        ++index;
    }
}

type semantic::resolve_call_id(call_id* node) {
    auto infer = resolve_identifier(node->get_id());
    if (infer.is_error()) {
        return infer;
    }
    if (node->get_generic_types() && !infer.is_global) {
        rp.report(node,
            "non global symbol \"" + node->get_id()->get_name() +
            "\" cannot be generic"
        );
    }
    if (node->get_generic_types()) {
        std::vector<type> types;
        for (auto i : node->get_generic_types()->get_types()) {
            types.push_back(tr.resolve(i));
        }
        std::string name = node->get_id()->get_name();
        name += "<";
        for (const auto& i : types) {
            name += i.full_path_name();
            for (auto j = 0; j < i.pointer_depth; ++j) {
                name += "*";
            }
            name += ",";
        }
        if (name.back()==',') {
            name.pop_back();
        }
        name += ">";
        
        if (!ctx.global.domain.count(infer.loc_file)) {
            rp.report(node, "namespace in \"" + infer.loc_file + "\" not found");
            return infer;
        }

        const auto& dm = ctx.get_domain(infer.loc_file);
        if (dm.structs.count(name) || dm.functions.count(name)) {
            infer.name = node->get_id()->get_name();
            infer.generics = types;
        } else {
            rp.report(node, "error resolving generic symbol \"" + name + "\"");
        }
    }

    node->set_resolve_type(infer);
    return infer;
}

type semantic::resolve_call_func_args(const type& prev, call_func_args* node) {
    // global function call
    if (prev.is_global_func) {
        const auto& domain = ctx.get_domain(prev.loc_file);
        if (!domain.functions.count(prev.name_for_search())) {
            rp.report(node,
                "cannot find function \"" + prev.name_for_search() + "\""
            );
            return type::error_type();
        }
        const auto& func = domain.functions.at(prev.name_for_search());
        check_static_call_args(func, node);
        node->set_resolve_type(func.return_type);
        return func.return_type;
    }
    // static method call of primitive type
    if (prev.prm_info.flag_is_static) {
        if (!ctx.global.primitives.count(prev.name_for_search())) {
            rp.report(node,
                "cannot call static method of primitive type \"" +
                prev.name_for_search() + "\""
            );
            return type::error_type();
        }
        const auto& p = ctx.global.primitives.at(prev.name_for_search());
        const auto& method = p.static_methods.at(prev.prm_info.method_name);
        check_static_call_args(method, node);
        node->set_resolve_type(method.return_type);
        return method.return_type;
    }
    // static method call of struct
    if (prev.stm_info.flag_is_static) {
        const auto& domain = ctx.get_domain(prev.loc_file);
        if (domain.structs.count(prev.name_for_search())) {
            const auto& st = domain.structs.at(prev.name_for_search());
            const auto& method = st.static_method.at(prev.stm_info.method_name);
            check_static_call_args(method, node);
            node->set_resolve_type(method.return_type);
            return method.return_type;
        } else if (domain.tagged_unions.count(prev.name_for_search())) {
            const auto& un = domain.tagged_unions.at(prev.name_for_search());
            const auto& method = un.static_method.at(prev.stm_info.method_name);
            check_static_call_args(method, node);
            node->set_resolve_type(method.return_type);
            return method.return_type;
        }
        
        rp.report(node,
            "cannot find \"" + prev.name_for_search() + "\""
        );
        return type::error_type();
    }
    // method call of struct
    if (prev.stm_info.flag_is_normal) {
        const auto& domain = ctx.get_domain(prev.loc_file);
        if (domain.structs.count(prev.name_for_search())) {
            const auto& st = domain.structs.at(prev.name_for_search());
            const auto& method = st.method.at(prev.stm_info.method_name);
            check_method_call_args(method, node);
            node->set_resolve_type(method.return_type);
            return method.return_type;
        } else if (domain.tagged_unions.count(prev.name_for_search())) {
            const auto& un = domain.tagged_unions.at(prev.name_for_search());
            const auto& method = un.method.at(prev.stm_info.method_name);
            check_method_call_args(method, node);
            node->set_resolve_type(method.return_type);
            return method.return_type;
        }

        rp.report(node,
            "cannot find \"" + prev.name_for_search() + "\""
        );
        return type::error_type();
    }

    rp.report(node, "cannot call non-function");
    return type::error_type();
}

type semantic::resolve_call_index(const type& prev, call_index* node) {
    if (prev.is_global) {
        rp.report(node,
            "cannot get index from global symbol \"" + prev.to_string() + "\""
        );
        return type::error_type();
    }
    if (!prev.is_pointer()) {
        rp.report(node,
            "cannot get index from \"" + prev.to_string() + "\""
        );
        return type::error_type();
    }

    auto index_infer = resolve_expression(node->get_index());
    if (!index_infer.is_error()) {
        if (!index_infer.is_integer() || index_infer.is_pointer()) {
            rp.report(node,
                "cannot get index with \"" + index_infer.to_string() + "\""
            );
        }
    }

    auto result = prev.get_ref_copy();
    // array type, remove array flag to make it mutable
    if (result.is_array) {
        result.is_array = false;
    }
    node->set_resolve_type(result);
    return result;
}

type semantic::resolve_initializer(const type& prev, initializer* node) {
    if (!prev.is_global) {
        rp.report(node, "need a global symbol to initialize");
        return type::error_type();
    }

    if (prev.loc_file.empty()) {
        rp.report(node, "basic type cannot be initialized as a struct");
        return type::error_type();
    }

    const auto& domain = ctx.get_domain(prev.loc_file);
    if (domain.structs.count(prev.name_for_search())) {
        return resolve_struct_initializer(
            domain.structs.at(prev.name_for_search()),
            prev,
            node
        );
    } else if (domain.tagged_unions.count(prev.name_for_search())) {
        return resolve_tagged_union_initializer(
            domain.tagged_unions.at(prev.name_for_search()),
            prev,
            node
        );
    }

    rp.report(node,
        "\"" + prev.name_for_search() +
        "\" is not struct or tagged union, cannot initialize"
    );
    return type::error_type();
}

type semantic::resolve_struct_initializer(const colgm_struct& st,
                                          const type& prev,
                                          initializer* node) {
    auto copy = prev;
    copy.is_global = false;
    node->set_resolve_type(copy);

    for (auto i : node->get_pairs()) {
        const auto& field = i->get_field()->get_name();
        if (!st.field.count(field)) {
            rp.report(i,
                "cannot find field \"" + field +
                "\" in \"" + prev.name_for_search() + "\""
            );
            continue;
        }

        const auto infer = resolve_expression(i->get_value());
        i->get_value()->set_resolve_type(infer);
        const auto& expect = st.field.at(field);
        // error type means the related error is reported before
        if (infer.is_error()) {
            continue;
        }
        if (infer != expect) {
            if (!check_can_be_converted(i->get_value(), expect)) {
                rp.report(i,
                    "expect \"" + expect.to_string() +
                    "\" but get \"" + infer.to_string() + "\""
                );
                continue;
            }
        }

        // foo { bar: [0, 1, 2] } is not allowed
        if (infer.is_array) {
            rp.report(i,
                "cannot initialize array, but it's already filled with 0 by default"
            );
        }
    }

    return copy;
}

type semantic::resolve_tagged_union_initializer(const colgm_tagged_union& un,
                                                const type& prev,
                                                initializer* node) {
    auto copy = prev;
    copy.is_global = false;
    node->set_resolve_type(copy);

    if (node->get_pairs().size() > 1) {
        rp.report(node, "cannot initialize tagged union with more than one member");
        return copy;
    } else if (node->get_pairs().empty()) {
        rp.report(node, "tagged union's initializer cannot be empty");
        return copy;
    }

    auto i = node->get_pairs()[0];
    const auto& member = i->get_field()->get_name();
    if (!un.member.count(member)) {
        rp.report(i,
            "cannot find member \"" + member +
            "\" in \"" + prev.name_for_search() + "\""
        );
        return copy;
    }

    const auto infer = resolve_expression(i->get_value());
    i->get_value()->set_resolve_type(infer);
    const auto& expect = un.member.at(member);
    // error type means the related error is reported before
    if (infer.is_error()) {
        return copy;
    }
    if (infer != expect) {
        if (!check_can_be_converted(i->get_value(), expect)) {
            rp.report(i,
                "expect \"" + expect.to_string() +
                "\" but get \"" + infer.to_string() + "\""
            );
            return copy;
        }
    }

    // foo { bar: [0, 1, 2] } is not allowed
    if (infer.is_array) {
        rp.report(i,
            "cannot initialize array, but it's already filled with 0 by default"
        );
    }

    return copy;
}

type semantic::resolve_call_path(const type& prev, call_path* node) {
    if (!prev.is_global) {
        rp.report(node, "cannot get path from a value");
        return type::error_type();
    }

    // prev resolved type is a primitive type
    if (prev.loc_file.empty()) {
        if (!ctx.global.primitives.count(prev.name_for_search())) {
            rp.report(node,
                "cannot get static method \"" + node->get_name() +
                "\" from \"" + prev.to_string() + "\""
            );
            return type::error_type();
        }
        const auto& p = ctx.global.primitives.at(prev.name_for_search());
        if (!p.static_methods.count(node->get_name())) {
            rp.report(node,
                "cannot get static method \"" + node->get_name() +
                "\" from \"" + prev.to_string() + "\""
            );
            return type::error_type();
        }
        auto infer = prev;
        infer.pointer_depth = 0;
        infer.is_global = true;
        infer.prm_info = {
            .flag_is_static = true,
            .flag_is_normal = false,
            .method_name = node->get_name()
        };
        return infer;
    }

    const auto& domain = ctx.get_domain(prev.loc_file);
    if (domain.structs.count(prev.name_for_search()) && prev.is_global) {
        const auto& st = domain.structs.at(prev.name_for_search());
        if (st.static_method.count(node->get_name())) {
            check_pub_static_method(node, node->get_name(), st);
            const auto res = struct_static_method_infer(prev, node->get_name());
            node->set_resolve_type(res);
            return res;
        } else if (st.method.count(node->get_name())) {
            rp.report(node,
                "\"" + node->get_name() +
                "\" in \"" + prev.name_for_search() + "\" is not static method"
            );
            return type::error_type();
        } else {
            rp.report(node,
                "cannot find static method \"" + node->get_name() +
                "\" in \"" + prev.name_for_search() + "\"",
                "maybe you mean \"" + st.fuzzy_match_field(node->get_name()) + "\"?"
            );
            return type::error_type();
        }
    } else if (domain.enums.count(prev.name_for_search()) && prev.is_global) {
        const auto& en = domain.enums.at(prev.name_for_search());
        if (en.members.count(node->get_name())) {
            auto res = prev;
            res.is_global = false;
            res.is_enum = true;
            node->set_resolve_type(res);
            return res;
        }
        rp.report(node,
            "cannot find enum member \"" + node->get_name() +
            "\" in \"" + prev.name_for_search() + "\""
        );
        return type::error_type();
    } else if (domain.tagged_unions.count(prev.name_for_search()) && prev.is_global) {
        const auto& tu = domain.tagged_unions.at(prev.name_for_search());
        if (tu.static_method.count(node->get_name())) {
            check_pub_static_method(node, node->get_name(), tu);
            const auto res = struct_static_method_infer(prev, node->get_name());
            node->set_resolve_type(res);
            return res;
        } else if (tu.method.count(node->get_name())) {
            rp.report(node,
                "\"" + node->get_name() +
                "\" in \"" + prev.name_for_search() + "\" is not static method"
            );
            return type::error_type();
        } else {
            rp.report(node,
                "cannot find static method \"" + node->get_name() +
                "\" in \"" + prev.name_for_search() + "\"",
                "maybe you mean \"" + tu.fuzzy_match_field(node->get_name()) + "\"?"
            );
            return type::error_type();
        }
    }

    rp.report(node,
        "cannot find path \"" + node->get_name() +
        "\" in \"" + prev.to_string() + "\""
    );
    return type::error_type();
}

type semantic::resolve_ptr_get_field(const type& prev, ptr_get_field* node) {
    if (prev.is_global) {
        rp.report(node,
            "cannot get field from global symbol \"" + prev.to_string() + "\""
        );
        return type::error_type();
    }
    if (prev.pointer_depth != 1) {
        rp.report(node->get_operator_location(),
            "cannot use \"->\" to get field from \"" +
            prev.to_string() + "\"",
            "use \".\" instead"
        );
        return type::error_type();
    }

    // prev resolved type is natvie type
    if (prev.loc_file.empty()) {
        rp.report(node,
            "cannot get method \"" + node->get_name() +
            "\" from \"" + prev.to_string() + "\""
        );
        return type::error_type();
    }

    const auto& domain = ctx.get_domain(prev.loc_file);
    if (domain.structs.count(prev.name_for_search())) {
        return resolve_struct_ptr_get_field(
            domain.structs.at(prev.name_for_search()),
            prev,
            node
        );
    } else if (domain.tagged_unions.count(prev.name_for_search())) {
        return resolve_tagged_union_ptr_get_field(
            domain.tagged_unions.at(prev.name_for_search()),
            prev,
            node
        );
    }

    rp.report(node, "cannot get field from \"" + prev.full_path_name() + "\"");
    return type::error_type();
}

type semantic::resolve_struct_ptr_get_field(const colgm_struct& struct_self,
                                            const type& prev,
                                            ptr_get_field* node) {
    if (struct_self.field.count(node->get_name())) {
        node->set_resolve_type(struct_self.field.at(node->get_name()));
        return struct_self.field.at(node->get_name());
    }
    if (struct_self.method.count(node->get_name())) {
        check_pub_method(node, node->get_name(), struct_self);
        const auto res = struct_method_infer(prev, node->get_name());
        node->set_resolve_type(res);
        return res;
    }
    if (struct_self.static_method.count(node->get_name())) {
        rp.report(node,
            "method \"" + node->get_name() +
            "\" in \"" + prev.name_for_search() + "\" is static"
        );
        return type::error_type();
    }
    rp.report(node,
        "cannot find field \"" + node->get_name() +
        "\" in \"" + prev.name_for_search() + "\"",
        "maybe you mean \"" + struct_self.fuzzy_match_field(node->get_name()) + "\"?"
    );
    return type::error_type();
}

type semantic::resolve_tagged_union_ptr_get_field(const colgm_tagged_union& un,
                                                  const type& prev,
                                                  ptr_get_field* node) {
    if (un.member.count(node->get_name())) {
        node->set_resolve_type(un.member.at(node->get_name()));
        return un.member.at(node->get_name());
    }
    if (un.method.count(node->get_name())) {
        check_pub_method(node, node->get_name(), un);
        const auto res = struct_method_infer(prev, node->get_name());
        node->set_resolve_type(res);
        return res;
    }
    if (un.static_method.count(node->get_name())) {
        rp.report(node,
            "method \"" + node->get_name() +
            "\" in \"" + prev.name_for_search() + "\" is static"
        );
        return type::error_type();
    }
    rp.report(node,
        "cannot find field \"" + node->get_name() +
        "\" in \"" + prev.name_for_search() + "\"",
        "maybe you mean \"" + un.fuzzy_match_field(node->get_name()) + "\"?"
    );
    return type::error_type();
}

type semantic::resolve_call(call* node) {
    auto infer = resolve_call_id(node->get_head());
    if (infer.is_error()) {
        return infer;
    }

    // resolve call chain
    for (auto i : node->get_chain()) {
        if (i->get_ast_type() != ast_type::ast_call_func_args &&
            infer.is_function()) {
            rp.report(i, "function should be called before");
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
    if (infer.is_function()) {
        rp.report(node, "function should be called here");
        return type::error_type();
    }
    node->set_resolve_type(infer);
    if (infer.is_global) {
        rp.report(node,
            "get global symbol \"" + infer.to_string() + "\", " +
            "but not an instance"
        );
    }
    return infer;
}

bool semantic::check_valid_left_value(expr* node) {
    if (!node->is(ast_type::ast_call)) {
        rp.report(node, "bad left value: should be a call");
        return false;
    }
    const auto mem_get_node = reinterpret_cast<call*>(node);
    for (auto i : mem_get_node->get_chain()) {
        if (i->is(ast_type::ast_initializer)) {
            rp.report(i, "bad left value: should not contain initializer");
            return false;
        }
        if ((i->is(ast_type::ast_call_path) ||
            i->is(ast_type::ast_call_func_args)) &&
            i == mem_get_node->get_chain().back()) {
            rp.report(i, "bad left value: should not end with function call");
            return false;
        }
    }

    expr* seg = nullptr;
    bool maybe_invalid_assignment = false;
    for (auto i : mem_get_node->get_chain()) {
        if (i->is(ast_type::ast_call_func_args) &&
            !i->get_resolve().is_pointer()) {
            seg = i;
            maybe_invalid_assignment = true;
        } else if (i->is(ast_type::ast_ptr_get_field)) {
            maybe_invalid_assignment = false;
        }
    }
    if (maybe_invalid_assignment && seg) {
        rp.warn(seg, "function returning non-pointer type,"
                     " will do shallow copy on return,"
                     " may cause invalid assignment");
    }
    return true;
}

void semantic::check_mutable_left_value(expr* node, const type& lt) {
    if (lt.is_const && !lt.is_pointer()) {
        rp.report(node, "cannot assign to \"const " + lt.to_string() + "\"");
    } else if (lt.is_array) {
        rp.report(node, "cannot change array \"" + lt.to_string() + "\"");
    }
    return;
}

type semantic::resolve_assignment(assignment* node) {
    const auto left = resolve_expression(node->get_left());
    // check left value is valid for assignment
    if (!check_valid_left_value(node->get_left())) {
        return type::error_type();
    }
    // check left value is mutable
    check_mutable_left_value(node->get_left(), left);
    const auto right = resolve_expression(node->get_right());
    if (left.is_error() || right.is_error()) {
        return type::error_type();
    }
    switch(node->get_type()) {
        case assignment::kind::andeq:
        case assignment::kind::xoreq:
        case assignment::kind::oreq:
            if (!left.can_bitwise_calculate()) {
                rp.report(node->get_left(),
                    "bitwise operator cannot be used on \"" +
                    left.to_string() + "\""
                );
                return type::restrict_type();
            }
            if (!right.can_bitwise_calculate()) {
                rp.report(node->get_right(),
                    "bitwise operator cannot be used on \"" +
                    right.to_string() + "\""
                );
                return type::restrict_type();
            }
        default: break;
    }
    if (left.is_pointer() && right.is_pointer()) {
        if (left != right && !check_can_be_converted(node->get_right(), left)) {
            rp.report(node,
                "cannot calculate \"" + left.to_string() +
                "\" and \"" + right.to_string() + "\""
            );
        }
    } else if (left != right) {
        if (!check_can_be_converted(node->get_right(), left)) {
            rp.report(node,
                "get \"" + right.to_string() +
                "\" but expect \"" + left.to_string() + "\""
            );
        }
    }

    // only = is allowed to be applied on enums
    if (ctx.search_symbol_kind(left)==sym_kind::enum_kind &&
        node->get_type()!=assignment::kind::eq) {
        rp.report(node, "cannot calculate enum \"" + left.to_string() + "\"");
        return type::restrict_type();
    }
    return type::restrict_type();
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
    case ast_type::ast_array_list:
        return resolve_array_list(reinterpret_cast<array_list*>(node));
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
    if (ctx.find_local(name)) {
        rp.report(node, "redefinition of variable \"" + name + "\"");
        return;
    }
    if (ctx.get_domain(node->get_file()).global_symbol.count(name)) {
        rp.report(node, "variable \"" + name + "\" conflicts with global symbol");
        return;
    }
    // must have init value!
    if (!node->get_init_value()) {
        rp.report(node, "expect initial value");
        return;
    }
    // no type declaration
    if (!node->get_type()) {
        const auto real_type = resolve_expression(node->get_init_value());
        if (real_type.is_empty_array()) {
            rp.report(node, "expect at least one element to infer type");
        }
        node->set_resolve_type(real_type);
        ctx.add_local(name, real_type);
        check_defined_variable_is_void(node, real_type);
        return;
    }

    // with type declaration
    const auto expected_type = tr.resolve(node->get_type());
    const auto real_type = resolve_expression(node->get_init_value());
    if (expected_type.is_array || real_type.is_array) {
        if (expected_type != real_type && !real_type.is_empty_array()) {
            rp.report(node,
                "expected \"" + expected_type.array_type_to_string() +
                "\", but get \"" + real_type.array_type_to_string() + "\""
            );
        } else if (expected_type.is_array &&
                   real_type.array_length > expected_type.array_length) {
            rp.report(node,
                "expected at most " + std::to_string(expected_type.array_length) +
                "element(s), but get " + std::to_string(real_type.array_length) + ""
            );
        }
        // sync the array length
        if (expected_type.is_array) {
            node->get_init_value()->set_resolve_type(expected_type);
        }
    } else if (expected_type.is_pointer() && real_type.is_pointer()) {
        if (expected_type != real_type &&
            !check_can_be_converted(node->get_init_value(), expected_type)) {
            rp.warn(node,
                "expected \"" + expected_type.to_string() +
                "\", but get \"" + real_type.to_string() + "\""
            );
        }
    } else if (expected_type != real_type) {
        if (!check_can_be_converted(node->get_init_value(), expected_type)) {
            rp.report(node->get_type(),
                "expected \"" + expected_type.to_string() +
                "\", but get \"" + real_type.to_string() + "\""
            );
        }
    }

    // if immutable, make sure the type is correct
    if (real_type.is_const) {
        node->set_resolve_type(real_type);
        ctx.add_local(name, real_type);
        check_defined_variable_is_void(node, real_type);
    } else {
        node->set_resolve_type(expected_type);
        ctx.add_local(name, expected_type);
        check_defined_variable_is_void(node, expected_type);
    }
}

void semantic::check_defined_variable_is_void(definition* node, const type& t) {
    if (!t.is_void()) {
        return;
    }
    rp.report(node, "cannot define variable with void type");
}

void semantic::resolve_if_stmt(if_stmt* node, const colgm_func& func_self) {
    if (node->get_condition()) {
        const auto infer = resolve_expression(node->get_condition());
        if (infer!=type::bool_type() && infer!=type::error_type()) {
            rp.report(node->get_condition(),
                "condition should be \"bool\" type but get \"" +
                infer.to_string() + "\""
            );
        }
    }
    if (node->get_block()) {
        resolve_code_block(node->get_block(), func_self);
    }
}

void semantic::resolve_cond_stmt(cond_stmt* node, const colgm_func& func_self) {
    for (auto i : node->get_stmts()) {
        resolve_if_stmt(i, func_self);
    }
}

bool semantic::check_is_match_default(expr* node) {
    if (!node->is(ast_type::ast_call)) {
        return false;
    }
    auto call_node = reinterpret_cast<call*>(node);
    if (call_node->get_chain().size() != 0) {
        return false;
    }

    const auto& name = call_node->get_head()->get_id()->get_name();
    return name == "_";
}

bool semantic::check_is_enum_literal(expr* node) {
    if (!node->is(ast_type::ast_call)) {
        return false;
    }
    auto call_node = reinterpret_cast<call*>(node);
    if (call_node->get_chain().size() != 1) {
        return false;
    }

    const auto& name = call_node->get_head()->get_id()->get_name();
    if (!ctx.global_symbol().count(name)) {
        return false;
    }
    if (ctx.global_symbol().at(name).kind != sym_kind::enum_kind) {
        return false;
    }

    if (!call_node->get_chain()[0]->is(ast_type::ast_call_path)) {
        return false;
    }
    return true;
}

size_t semantic::get_enum_literal_value(expr* node, const type& infer) {
    const auto call_node = reinterpret_cast<call*>(node);
    const auto name_node = call_node->get_head()->get_id();
    const auto& name = name_node->get_name();

    if (!ctx.global.domain.count(infer.loc_file)) {
        return SIZE_MAX;
    }

    const auto& dm = ctx.get_domain(infer.loc_file);
    if (!dm.enums.count(name)) {
        return SIZE_MAX;
    }
    
    const auto& em = dm.enums.at(name);
    const auto path_node = reinterpret_cast<call_path*>(call_node->get_chain()[0]);
    const auto& ename = path_node->get_name();
    if (!em.members.count(ename)) {
        return SIZE_MAX;
    }
    return em.members.at(ename);
}

void semantic::resolve_match_stmt(match_stmt* node, const colgm_func& func_self) {
    const auto infer = resolve_expression(node->get_value());
    // check error type
    if (infer.is_error()) {
        return;
    }

    if (ctx.search_symbol_kind(infer) != sym_kind::enum_kind &&
        ctx.search_symbol_kind(infer) != sym_kind::tagged_union_kind) {
        rp.report(node->get_value(), "match value should be enum or tagged union");
        return;
    }

    if (ctx.search_symbol_kind(infer) == sym_kind::enum_kind &&
        infer.is_pointer()) {
        rp.report(node->get_value(), "match value should not be pointer");
    }

    if (node->get_cases().empty()) {
        rp.report(node, "expect at least one case");
        return;
    }

    if (ctx.search_symbol_kind(infer) == sym_kind::enum_kind) {
        resolve_match_stmt_for_enum(node, func_self, infer);
    } else if (ctx.search_symbol_kind(infer) == sym_kind::tagged_union_kind) {
        resolve_match_stmt_for_tagged_union(node, func_self, infer);
    }
}

void semantic::resolve_match_stmt_for_enum(match_stmt* node,
                                           const colgm_func& func_self,
                                           const type& infer) {
    bool default_found = false;
    std::unordered_set<size_t> used_values;
    for (auto i : node->get_cases()) {
        if (check_is_match_default(i->get_value())) {
            i->get_value()->set_resolve_type(type::default_match_type());
            default_found = true;
            continue;
        }

        const auto case_node = i->get_value();
        const auto case_infer = resolve_expression(case_node);
        case_node->set_resolve_type(case_infer);
        if (case_infer != infer) {
            // do not report again if case infer is error type
            if (!case_infer.is_error()) {
                rp.report(case_node,
                    "case value should be \"" + infer.to_string() +
                    "\" type but get \"" + case_infer.to_string() + "\""
                );
            }
            continue;
        }

        if (!check_is_enum_literal(case_node)) {
            rp.report(case_node, "case value should be enum literal");
            continue;
        }
    
        const auto value = get_enum_literal_value(case_node, infer);
        if (value == SIZE_MAX) {
            // unreachable
            continue;
        }
        if (used_values.count(value)) {
            rp.report(case_node, "duplicate enum literal value");
        }
        used_values.insert(value);
    }

    // resolve blocks after labels
    for (auto i : node->get_cases()) {
        resolve_code_block(i->get_block(), func_self);
    }

    if (default_found) {
        return;
    }

    // check unused values if default is not used
    if (!ctx.global.domain.count(infer.loc_file)) {
        return;
    }
    const auto& dm = ctx.get_domain(infer.loc_file);
    if (!dm.enums.count(infer.name)) {
        return;
    }
    const auto& em = dm.enums.at(infer.name);
    auto generated_warning = std::string("");
    auto unused_counter = 0;

    for (auto& i : em.members) {
        if (used_values.count(i.second)) {
            continue;
        }
        if (generated_warning.length()) {
            generated_warning += ", ";
        }
        unused_counter++;
        if (unused_counter >= 3) {
            generated_warning += "...";
            break;
        }
        generated_warning += i.first;
    }

    if (generated_warning.length()) {
        rp.warn(node, "match statement with enum \"" + infer.to_string() +
            "\" has unused member" + (unused_counter > 1? "s":"") +
            ": " + generated_warning
        );
    }
}

bool semantic::check_is_tagged_union_member(expr* node, const colgm_tagged_union& un) {
    if (!node->is(ast_type::ast_call)) {
        // unreachable
        return false;
    }

    auto call_node = static_cast<call*>(node);
    const auto& name = call_node->get_head()->get_id()->get_name();

    // means this tag is not referenced from enum
    // so the call head's identifier is the tag
    if (un.ref_enum_type.is_empty()) {
        if (call_node->get_chain().size() != 0) {
            return false;
        }
        return un.member.count(name);
    }

    if (name != un.ref_enum_type.name) {
        return false;
    }

    if (call_node->get_chain().size() != 1) {
        return false;
    }

    auto chain = call_node->get_chain()[0];
    if (!chain->is(ast_type::ast_call_path)) {
        return false;
    }

    return un.member.count(static_cast<call_path*>(chain)->get_name());
}

std::string semantic::get_tagged_union_label(expr* node, const colgm_tagged_union& un) {
    auto call_node = static_cast<call*>(node);
    const auto& name = call_node->get_head()->get_id()->get_name();

    // means this tag is not referenced from enum
    // so the call head's identifier is the tag
    if (un.ref_enum_type.is_empty()) {
        return name;
    }

    auto chain = call_node->get_chain()[0];
    return static_cast<call_path*>(chain)->get_name();
}

void semantic::resolve_match_stmt_for_tagged_union(match_stmt* node,
                                                   const colgm_func& func_self,
                                                   const type& infer) {
    bool default_found = false;
    std::unordered_set<std::string> used_values;

    if (infer.pointer_depth > 1) {
        rp.report(node->get_value(),
            "cannot match \"" + infer.full_path_name_with_pointer() + "\""
        );
    }

    const auto& dm = ctx.get_domain(infer.loc_file);
    const auto& un = dm.tagged_unions.at(infer.name);

    for (auto i : node->get_cases()) {
        if (check_is_match_default(i->get_value())) {
            i->get_value()->set_resolve_type(type::default_match_type());
            default_found = true;
            continue;
        }
        const auto case_node = i->get_value();
        case_node->set_resolve_type(infer);

        if (!check_is_tagged_union_member(case_node, un)) {
            rp.report(case_node,
                "cannot find this tag in this union"
            );
            continue;
        }

        const auto label = get_tagged_union_label(case_node, un);
        if (used_values.count(label)) {
            rp.report(case_node, "duplicate tag");
        }
        used_values.insert(label);
    }

    // resolve blocks after labels
    for (auto i : node->get_cases()) {
        resolve_code_block(i->get_block(), func_self);
    }

    if (default_found) {
        return;
    }

    auto generated_warning = std::string("");
    auto unused_counter = 0;

    for (auto& i : un.member) {
        if (used_values.count(i.first)) {
            continue;
        }
        if (generated_warning.length()) {
            generated_warning += ", ";
        }
        unused_counter++;
        if (unused_counter >= 3) {
            generated_warning += "...";
            break;
        }
        generated_warning += i.first;
    }

    if (generated_warning.length()) {
        rp.warn(node, "match statement with union \"" + infer.to_string() +
            "\" has unused tag" + (unused_counter > 1? "s":"") +
            ": " + generated_warning,
            "complete unused branch or use \"_\" default branch instead"
        );
    }
}

void semantic::resolve_while_stmt(while_stmt* node, const colgm_func& func_self) {
    const auto infer = resolve_expression(node->get_condition());
    if (infer!=type::bool_type() && infer!=type::error_type()) {
        rp.report(node->get_condition(),
            "condition should be \"bool\" type but get \"" +
            infer.to_string() + "\""
        );
    }
    if (node->get_block()) {
        ++in_loop_level;
        resolve_code_block(node->get_block(), func_self);
        --in_loop_level;
    }
}

void semantic::resolve_for_stmt(for_stmt* node, const colgm_func& func_self) {
    ctx.push_level();
    if (node->get_init()) {
        resolve_definition(node->get_init(), func_self);
    }
    if (node->get_condition()) {
        const auto infer = resolve_expression(node->get_condition());
        if (infer!=type::bool_type() && infer!=type::error_type()) {
            rp.report(node->get_condition(),
                "condition should be \"bool\" type but get \"" +
                infer.to_string() + "\""
            );
        }
    }
    if (node->get_update()) {
        resolve_expression(node->get_update());
    }
    if (node->get_block()) {
        ++in_loop_level;
        resolve_code_block(node->get_block(), func_self);
        --in_loop_level;
    }
    ctx.pop_level();
}

void semantic::lowering_forindex(forindex* node) {
    auto lowered_init = new definition(
        node->get_variable()->get_location(),
        node->get_variable()->get_name()
    );
    lowered_init->set_type(new type_def(node->get_variable()->get_location()));
    lowered_init->get_type()->set_name(new identifier(
        node->get_variable()->get_location(),
        "u64"
    ));
    lowered_init->set_init_value(new number_literal(
        node->get_variable()->get_location(),
        "0"
    ));
    node->set_lowered_init(lowered_init);

    auto lowered_condition_lhs = new call(node->get_container()->get_location());
    lowered_condition_lhs->set_head(new call_id(node->get_container()->get_location()));
    lowered_condition_lhs->get_head()->set_id(new identifier(
        node->get_variable()->get_location(),
        node->get_variable()->get_name()
    ));

    auto lowered_condition_rhs = node->get_container()->clone();
    if (node->get_container()->get_resolve().is_pointer()) {
        lowered_condition_rhs->add_chain(new ptr_get_field(
            node->get_container()->get_location(),
            node->get_container()->get_location(),
            "iter_size"
        ));
    } else {
        lowered_condition_rhs->add_chain(new get_field(
            node->get_container()->get_location(),
            node->get_container()->get_location(),
            "iter_size"
        ));
    }
    lowered_condition_rhs->add_chain(new call_func_args(node->get_container()->get_location()));

    auto lowered_condition = new binary_operator(node->get_container()->get_location());
    lowered_condition->set_opr(binary_operator::kind::less);
    lowered_condition->set_left(lowered_condition_lhs);
    lowered_condition->set_right(lowered_condition_rhs);
    node->set_lowered_condition(lowered_condition);

    auto lowered_update_lhs = lowered_condition_lhs->clone();
    auto lowered_update_rhs = new number_literal(node->get_container()->get_location(), "1");
    auto lowered_update = new assignment(node->get_container()->get_location());
    lowered_update->set_type(assignment::kind::addeq);
    lowered_update->set_left(lowered_update_lhs);
    lowered_update->set_right(lowered_update_rhs);
    node->set_lowered_update(lowered_update);
}

void semantic::resolve_forindex(forindex* node, const colgm_func& func_self) {
    resolve_expression(node->get_container());
    lowering_forindex(node);

    ctx.push_level();
    resolve_definition(node->get_lowered_init(), func_self);
    resolve_expression(node->get_lowered_condition());
    resolve_expression(node->get_lowered_update());
    if (node->get_body()) {
        ++in_loop_level;
        resolve_code_block(node->get_body(), func_self);
        --in_loop_level;
    }
    ctx.pop_level();
}

void semantic::lowering_foreach(foreach* node) {
    auto lowered_init = new definition(
        node->get_variable()->get_location(),
        node->get_variable()->get_name()
    );
    auto lowered_init_rhs = node->get_container()->clone();
    lowered_init->set_init_value(lowered_init_rhs);
    if (node->get_container()->get_resolve().is_pointer()) {
        lowered_init_rhs->add_chain(new ptr_get_field(
            node->get_container()->get_location(),
            node->get_container()->get_location(),
            "iter"
        ));
    } else {
        lowered_init_rhs->add_chain(new get_field(
            node->get_container()->get_location(),
            node->get_container()->get_location(),
            "iter"
        ));
    }
    lowered_init_rhs->add_chain(new call_func_args(
        node->get_container()->get_location()
    ));
    node->set_lowered_init(lowered_init);

    auto lowered_condition_value = new call(node->get_container()->get_location());
    lowered_condition_value->set_head(new call_id(
        node->get_container()->get_location()
    ));
    lowered_condition_value->get_head()->set_id(new identifier(
        node->get_container()->get_location(),
        node->get_variable()->get_name()
    ));
    lowered_condition_value->add_chain(new get_field(
        node->get_container()->get_location(),
        node->get_container()->get_location(),
        "is_end"
    ));
    lowered_condition_value->add_chain(new call_func_args(
        node->get_container()->get_location()
    ));
    auto lowered_condition = new unary_operator(
        node->get_container()->get_location()
    );
    lowered_condition->set_opr(unary_operator::kind::lnot);
    lowered_condition->set_value(lowered_condition_value);
    node->set_lowered_condition(lowered_condition);

    auto lowered_update_lhs = new call(node->get_container()->get_location());
    lowered_update_lhs->set_head(new call_id(
        node->get_container()->get_location()
    ));
    lowered_update_lhs->get_head()->set_id(new identifier(
        node->get_container()->get_location(),
        node->get_variable()->get_name()
    ));
    auto lowered_update_rhs = new call(node->get_container()->get_location());
    lowered_update_rhs->set_head(new call_id(
        node->get_container()->get_location()
    ));
    lowered_update_rhs->get_head()->set_id(new identifier(
        node->get_container()->get_location(),
        node->get_variable()->get_name()
    ));
    lowered_update_rhs->add_chain(new get_field(
        node->get_container()->get_location(),
        node->get_container()->get_location(),
        "next"
    ));
    lowered_update_rhs->add_chain(new call_func_args(
        node->get_container()->get_location()
    ));
    auto lowered_update = new assignment(node->get_container()->get_location());
    lowered_update->set_type(assignment::kind::eq);
    lowered_update->set_left(lowered_update_lhs);
    lowered_update->set_right(lowered_update_rhs);
    node->set_lowered_update(lowered_update);
}

void semantic::resolve_foreach(foreach* node, const colgm_func& func_self) {
    resolve_expression(node->get_container());
    lowering_foreach(node);

    ctx.push_level();
    resolve_definition(node->get_lowered_init(), func_self);
    resolve_expression(node->get_lowered_condition());
    resolve_expression(node->get_lowered_update());
    if (node->get_body()) {
        ++in_loop_level;
        resolve_code_block(node->get_body(), func_self);
        --in_loop_level;
    }
    ctx.pop_level();
}

void semantic::resolve_in_stmt_expr(in_stmt_expr* node, const colgm_func& func_self) {
    node->set_resolve_type(resolve_expression(node->get_expr()));
}

void semantic::resolve_ret_stmt(ret_stmt* node, const colgm_func& func_self) {
    if (!node->get_value() && func_self.return_type != type::void_type()) {
        rp.report(node,
            "expected return type \"" + func_self.return_type.to_string() +
            "\" but get \"void\""
        );
        return;
    }
    if (!node->get_value()) {
        return;
    }
    const auto infer = resolve_expression(node->get_value());
    if (infer.is_error()) {
        return;
    }

    if (infer != func_self.return_type &&
        !check_can_be_converted(node->get_value(), func_self.return_type)) {
        rp.report(node,
            "expected return type \"" + func_self.return_type.to_string() +
            "\" but get \"" + infer.to_string() + "\""
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
    case ast_type::ast_forindex:
        resolve_forindex(reinterpret_cast<forindex*>(node), func_self);
        break;
    case ast_type::ast_foreach:
        resolve_foreach(reinterpret_cast<foreach*>(node), func_self);
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
            rp.report(node, "this statement should be used inside a loop");
        }
        break;
    default:
        rp.unreachable(node);
        break;
    }
}

void semantic::resolve_code_block(code_block* node, const colgm_func& func_self) {
    // should not be called to resolve function's top code block
    ctx.push_level();
    for (auto i : node->get_stmts()) {
        resolve_statement(i, func_self);
        if (i->is(ast_type::ast_ret_stmt) ||
            i->is(ast_type::ast_continue_stmt) ||
            i->is(ast_type::ast_break_stmt)) {
            report_unreachable_statements(node);
            break;
        }
    }
    ctx.pop_level();
}

void semantic::resolve_global_func(func_decl* node) {
    if (node->get_generic_types()) {
        return; // do not resolve generic impl
    }
    const auto& domain = ctx.get_domain(ctx.this_file);
    if (!domain.functions.count(node->get_name())) {
        rp.report(node, "cannot find function \"" + node->get_name() + "\"");
        return;
    }
    const auto& func_self = domain.functions.at(node->get_name());
    if (!node->get_code_block()) {
        if (!func_self.is_extern) {
            rp.warn(node,
                "non-extern function \"" + node->get_name() +
                "\" is not implemented"
            );
        }
        return;
    }
    ctx.push_level();
    for (const auto& p : func_self.ordered_params) {
        ctx.add_local(p, func_self.params.at(p));
    }
    for (auto i : node->get_code_block()->get_stmts()) {
        resolve_statement(i, func_self);
        if (i->is(ast_type::ast_ret_stmt) ||
            i->is(ast_type::ast_continue_stmt) ||
            i->is(ast_type::ast_break_stmt)) {
            report_unreachable_statements(node->get_code_block());
            break;
        }
    }
    report_top_level_block_has_no_return(node->get_code_block(), func_self);
    ctx.pop_level();
}

void semantic::resolve_method(func_decl* node, const colgm_struct& struct_self) {
    if (!node->get_code_block()) {
        rp.report(node, "should be implemented here");
        return;
    }
    ctx.push_level();
    const auto& method_self = struct_self.method.count(node->get_name())
        ? struct_self.method.at(node->get_name())
        : struct_self.static_method.at(node->get_name());
    for (const auto& p : method_self.ordered_params) {
        ctx.add_local(p, method_self.params.at(p));
    }
    for (auto i : node->get_code_block()->get_stmts()) {
        resolve_statement(i, method_self);
        if (i->is(ast_type::ast_ret_stmt) ||
            i->is(ast_type::ast_continue_stmt) ||
            i->is(ast_type::ast_break_stmt)) {
            report_unreachable_statements(node->get_code_block());
            break;
        }
    }
    report_top_level_block_has_no_return(node->get_code_block(), method_self);
    ctx.pop_level();
}

void semantic::resolve_method(func_decl* node, const colgm_tagged_union& union_self) {
    if (!node->get_code_block()) {
        rp.report(node, "should be implemented here");
        return;
    }
    ctx.push_level();
    const auto& method_self = union_self.method.count(node->get_name())
        ? union_self.method.at(node->get_name())
        : union_self.static_method.at(node->get_name());
    for (const auto& p : method_self.ordered_params) {
        ctx.add_local(p, method_self.params.at(p));
    }
    for (auto i : node->get_code_block()->get_stmts()) {
        resolve_statement(i, method_self);
        if (i->is(ast_type::ast_ret_stmt) ||
            i->is(ast_type::ast_continue_stmt) ||
            i->is(ast_type::ast_break_stmt)) {
            report_unreachable_statements(node->get_code_block());
            break;
        }
    }
    report_top_level_block_has_no_return(node->get_code_block(), method_self);
    ctx.pop_level();
}

void semantic::resolve_impl(impl* node) {
    if (node->get_generic_types()) {
        return; // do not resolve generic impl
    }

    const auto& domain = node->is_redirected()
        ? ctx.get_domain(node->get_redirect_location())
        : ctx.get_domain(node->get_file());
    
    if (domain.structs.count(node->get_name())) {
        const auto& struct_self = domain.structs.at(node->get_name());
        impl_struct_name = node->get_name();
        for (auto i : node->get_methods()) {
            resolve_method(i, struct_self);
        }
        impl_struct_name = "";
        return;
    } else if (domain.tagged_unions.count(node->get_name())) {
        const auto& union_self = domain.tagged_unions.at(node->get_name());
        impl_struct_name = node->get_name();
        for (auto i : node->get_methods()) {
            resolve_method(i, union_self);
        }
        impl_struct_name = "";
        return;
    }

    rp.report(node,
        "cannot implement \"" + node->get_name() +
        "\", this struct or tagged union is not defined in the same file"
    );
}

void semantic::resolve_function_block(root* ast_root) {
    for (auto i : ast_root->get_decls()) {
        if (i->is(ast_type::ast_impl)) {
            resolve_impl(reinterpret_cast<impl*>(i));
        }
        if (i->is(ast_type::ast_func_decl)) {
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
    ctx.dump_tagged_unions();
    ctx.dump_functions();
}

}