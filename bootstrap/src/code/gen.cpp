#include "colgm.h"
#include "code/gen.h"
#include "sema/symbol.h"
#include "sema/basic.h"
#include "semantic.h"

namespace colgm {

std::string generator::type_convert(const type& t) {
    auto copy = t;
    if (basic_type_convert_mapper.count(copy.name)) {
        copy.name = basic_type_convert_mapper.at(copy.name);
    } else if (ctx.global_symbol.count(copy.name) &&
        ctx.global_symbol.at(copy.name).kind==symbol_kind::struct_kind) {
        copy.name = "%struct." + mangle_in_module_symbol(t.full_path_name());
    }
    return copy.to_string();
}

std::string generator::generate_type_string(type_def* node) {
    auto loc_file = node->get_location().file;
    if (ctx.global_symbol.count(node->get_name()->get_name())) {
        loc_file = ctx.global_symbol.at(node->get_name()->get_name()).loc_file;
    }
    return type_convert({
        node->get_name()->get_name(),
        loc_file,
        node->get_pointer_level()
    });
}

bool generator::visit_struct_decl(struct_decl* node) {
    auto new_struct = hir_struct(node->get_name(), node->get_location());
    for(auto i : node->get_fields()) {
        new_struct.add_field_type(generate_type_string(i->get_type()));
    }
    emit_struct_decl(new_struct);
    return true;
}

void generator::convert_parameter_to_pointer(func_decl* node) {
    for(auto i : node->get_params()->get_params()) {
        auto param_addr = new sir_alloca(
            i->get_name()->get_name(),
            generate_type_string(i->get_type())
        );
        ircode_block->add_alloca(param_addr);
        auto store_param_to_addr = new sir_store(
            generate_type_string(i->get_type()),
            i->get_name()->get_name() + ".param",
            i->get_name()->get_name()
        );
        ircode_block->add_stmt(store_param_to_addr);
    }
}

bool generator::visit_func_decl(func_decl* node) {
    ssa_temp_counter = 0;
    place_holder_label = 0;

    auto name = node->get_name();
    if (impl_struct_name.length()) {
        name = impl_struct_name + "." + name;
    }

    auto new_ir = new hir_func("@" + name);
    new_ir->set_return_type(generate_type_string(node->get_return_type()));
    // generate parameter list
    for(auto i : node->get_params()->get_params()) {
        new_ir->add_param(
            i->get_name()->get_name() + ".param",
            generate_type_string(i->get_type())
        );
    }
    if (node->get_code_block()) {
        emit_func_impls(new_ir);
        new_ir->set_code_block(new sir_block);
    } else {
        emit_func_decl(new_ir);
        return true;
    }
    ircode_block = new_ir->get_code_block();
    // convert parameter to pointer to avoid type issues in llvm ir
    convert_parameter_to_pointer(node);
    node->get_code_block()->accept(this);
    ircode_block = nullptr;
    return true;
}

bool generator::visit_impl_struct(impl_struct* node) {
    impl_struct_name = node->get_struct_name();
    for(auto i : node->get_methods()) {
        i->accept(this);
    }
    impl_struct_name = "";
    return true;
}

bool generator::visit_code_block(code_block* node) {
    for(auto i : node->get_stmts()) {
        i->accept(this);
        // we do not generate unreachable statements
        if (i->get_ast_type()==ast_type::ast_ret_stmt ||
            i->get_ast_type()==ast_type::ast_continue_stmt ||
            i->get_ast_type()==ast_type::ast_break_stmt) {
            break;
        }
    }
    return true;
}

void generator::call_expression_generation(call* node, bool need_address) {
    if (node->get_head()->get_ast_type()!=ast_type::ast_identifier) {
        unimplemented(node->get_head());
        return;
    }
    // call head may be the global function
    auto head = reinterpret_cast<identifier*>(node->get_head());
    if (ctx.global_symbol.count(head->get_name()) &&
        ctx.global_symbol.at(head->get_name()).kind==symbol_kind::func_kind) {
        call_function_symbol(head);
    } else {
        call_variable(head);
    }
    for(auto i : node->get_chain()) {
        i->accept(this);
    }
    if (value_stack.empty()) {
        report_stack_empty(node);
        return;
    }
    const auto source = value_stack.back();
    if (source.resolve_type==type::void_type()) {
        value_stack.pop_back();
        return;
    }
    // all of the source must be pointer
    if (source.resolve_type.pointer_depth<=0) {
        err.err("code", node->get_location(),
            "internal compiler bug: should be pointer."
        );
        return;
    }
    // need address flag, for assignment
    if (need_address) {
        return;
    }
    value_stack.pop_back();
    const auto temp = get_temp_variable();
    auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = source.resolve_type,
        .content = temp
    };
    result.resolve_type.pointer_depth--;
    value_stack.push_back(result);
    ircode_block->add_stmt(new sir_load(
        type_convert(result.resolve_type),
        source.content,
        temp
    ));
}

bool generator::visit_nil_literal(nil_literal* node) {
    const auto temp = get_temp_variable();
    ircode_block->add_stmt(new sir_nil(temp));
    // push value on stack
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = temp
    };
    value_stack.push_back(result);
    return true;
}

bool generator::visit_number_literal(number_literal* node) {
    const auto temp = get_temp_variable();
    ircode_block->add_stmt(new sir_number(
        node->get_number(),
        temp,
        type_convert(node->get_resolve()),
        node->get_resolve().is_integer()
    ));
    // push value on stack
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = temp
    };
    value_stack.push_back(result);
    return true;
}

bool generator::visit_string_literal(string_literal* node) {
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    if (irc.const_strings.count(node->get_string())) {
        ircode_block->add_stmt(new sir_string(
            node->get_string(),
            irc.const_strings.at(node->get_string()),
            result.content
        ));
    } else {
        ircode_block->add_stmt(new sir_string(
            node->get_string(),
            irc.const_strings.size(),
            result.content
        ));
        irc.const_strings.insert({node->get_string(), irc.const_strings.size()});
    }
    value_stack.push_back(result);
    return true;
}

bool generator::visit_char_literal(char_literal* node) {
    const auto temp = get_temp_variable();
    ircode_block->add_stmt(new sir_number(
        std::to_string(int64_t(node->get_char())),
        temp,
        type_convert(node->get_resolve()),
        node->get_resolve().is_integer()
    ));
    // push value on stack
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = temp
    };
    value_stack.push_back(result);
    return true;
}

bool generator::visit_bool_literal(bool_literal* node) {
    const auto temp = get_temp_variable();
    ircode_block->add_stmt(new sir_number(
        node->get_flag()? "1":"0",
        temp,
        "i1",
        true
    ));
    // push value on stack
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = temp
    };
    value_stack.push_back(result);
    return true;
}

void generator::call_function_symbol(identifier* node) {
    value_stack.push_back({
        .kind = value_kind::v_sfn,
        .resolve_type = node->get_resolve(),
        .content = node->get_name()
    });
}

void generator::call_variable(identifier* node) {
    // in fact we need to get the pointer to this variable
    auto resolve = node->get_resolve();
    resolve.pointer_depth++;
    // push pointer to the variable on top of the stack
    value_stack.push_back({
        .kind = value_kind::v_var,
        .resolve_type = resolve,
        .content = node->get_name()
    });
}

bool generator::visit_call(call* node) {
    call_expression_generation(node, false);
    return true;
}

bool generator::visit_call_index(call_index* node) {
    node->get_index()->accept(this);
    const auto index = value_stack.back();
    value_stack.pop_back();
    const auto source = value_stack.back();
    value_stack.pop_back();

    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();

    auto source_type_copy = source.resolve_type;
    if (source_type_copy.pointer_depth<2) {
        err.err("code", node->get_location(),
            "source pointer depth is less than 2."
        );
    }
    source_type_copy.pointer_depth--;
    ircode_block->add_stmt(new sir_load(
        type_convert(source_type_copy),
        source.content,
        temp_0
    ));
    auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = temp_1
    };
    // in fact, must be pointer...
    result.resolve_type.pointer_depth++;

    value_stack.push_back(result);
    source_type_copy.pointer_depth--;
    ircode_block->add_stmt(new sir_call_index(
        temp_0,
        temp_1,
        index.content,
        type_convert(source_type_copy),
        type_convert(index.resolve_type)
    ));
    return true;
}

void generator::generate_basic_method_call(call_field* node,
                                           const value& source) {
    const auto bsc = colgm_basic::mapper.at(source.resolve_type.name);
    if (!bsc->method.count(node->get_name())) {
        unreachable(node);
        return;
    }
    const auto name = "native." + source.resolve_type.name +
                        "." + node->get_name();
    if (!irc.used_basic_convert_method.count(source.resolve_type.name)) {
        irc.used_basic_convert_method.insert({
            source.resolve_type.name,
            {}
        });
    }
    irc.used_basic_convert_method.at(source.resolve_type.name).insert(
        bsc->method.at(node->get_name()).return_type.name
    );
    auto result = value {
        .kind = value_kind::v_sfn,
        .resolve_type = node->get_resolve(),
        .content = name
    };
    value_stack.push_back(result);
    // load pointer's content to a temporary variable
    auto self = value {
        .kind = value_kind::v_var,
        .resolve_type = source.resolve_type,
        .content = get_temp_variable()
    };
    self.resolve_type.pointer_depth--;
    ircode_block->add_stmt(new sir_load(
        type_convert(self.resolve_type),
        source.content,
        self.content
    ));
    value_stack.push_back(self); // self number
}

bool generator::visit_call_field(call_field* node) {
    const auto source = value_stack.back();
    value_stack.pop_back();
    // basic symbol, for example i8, i16, i32, i64
    if (source.resolve_type.loc_file.empty()) {
        generate_basic_method_call(node, source);
        return true;
    }
    if (!ctx.global.domain.count(source.resolve_type.loc_file)) {
        err.err("code", node->get_location(),
            "cannot find domain \"" + source.resolve_type.loc_file + "\"."
        );
        return true;
    }
    const auto& dom = ctx.global.domain.at(source.resolve_type.loc_file);
    if (!dom.structs.count(source.resolve_type.name)) {
        err.err("code", node->get_location(),
            "cannot find struct \"" + source.resolve_type.name + "\"."
        );
        return true;
    }
    const auto& st = dom.structs.at(source.resolve_type.name);
    if (st.field.count(node->get_name())) {
        for(usize i = 0; i<st.ordered_field.size(); ++i) {
            if (node->get_name()==st.ordered_field[i].name) {
                auto resolve = source.resolve_type;
                resolve.pointer_depth--;
                const auto temp = get_temp_variable();
                ircode_block->add_stmt(new sir_call_field(
                    temp,
                    source.content,
                    type_convert(resolve),
                    i
                ));
                auto result = value {
                    .kind = value_kind::v_var,
                    .resolve_type = st.ordered_field[i].symbol_type,
                    .content = temp
                };
                result.resolve_type.pointer_depth++;
                value_stack.push_back(result);
                break;
            }
        }
        return true;
    }
    if (st.method.count(node->get_name())) {
        auto result = value {
            .kind = value_kind::v_fn,
            .resolve_type = node->get_resolve(),
            .content = st.name + "." + node->get_name()
        };
        value_stack.push_back(result);
        value_stack.push_back(source); // self pointer
        return true;
    }
    unreachable(node);
    return true;
}

bool generator::visit_ptr_call_field(ptr_call_field* node) {
    const auto source = value_stack.back();
    value_stack.pop_back();
    if (!ctx.global.domain.count(source.resolve_type.loc_file)) {
        err.err("code", node->get_location(),
            "cannot find domain \"" + source.resolve_type.loc_file + "\"."
        );
        return true;
    }
    const auto& dom = ctx.global.domain.at(source.resolve_type.loc_file);
    if (!dom.structs.count(source.resolve_type.name)) {
        err.err("code", node->get_location(),
            "cannot find struct \"" + source.resolve_type.name + "\"."
        );
        return true;
    }
    const auto& st = dom.structs.at(source.resolve_type.name);
    if (st.field.count(node->get_name())) {
        for(usize i = 0; i<st.ordered_field.size(); ++i) {
            if (node->get_name()==st.ordered_field[i].name) {
                const auto temp_0 = get_temp_variable();
                const auto temp_1 = get_temp_variable();
                auto source_type_copy = source.resolve_type;
                source_type_copy.pointer_depth--;
                ircode_block->add_stmt(new sir_load(
                    type_convert(source_type_copy),
                    source.content,
                    temp_0
                ));
                source_type_copy.pointer_depth--;
                ircode_block->add_stmt(new sir_call_field(
                    temp_1,
                    temp_0,
                    type_convert(source_type_copy),
                    i
                ));
                auto result = value {
                    .kind = value_kind::v_var,
                    .resolve_type = node->get_resolve(),
                    .content = temp_1
                };
                result.resolve_type.pointer_depth++;
                value_stack.push_back(result);
                break;
            }
        }
        return true;
    }
    if (st.method.count(node->get_name())) {
        const auto temp_0 = get_temp_variable();
        auto source_type_copy = source.resolve_type;
        source_type_copy.pointer_depth--;
        ircode_block->add_stmt(new sir_load(
            type_convert(source_type_copy),
            source.content,
            temp_0
        ));
        auto result = value {
            .kind = value_kind::v_fn,
            .resolve_type = node->get_resolve(),
            .content = st.name + "." + node->get_name()
        };
        auto self = value {
            .kind = value_kind::v_var,
            .resolve_type = source_type_copy,
            temp_0
        };
        value_stack.push_back(result);
        value_stack.push_back(self); // self pointer
        return true;
    }
    unreachable(node);
    return true;
}

void generator::generate_basic_static_method(call_path* node,
                                             const value& source) {
    const auto bsc = colgm_basic::mapper.at(source.resolve_type.name);
    if (!bsc->static_method.count(node->get_name())) {
        unreachable(node);
        return;
    }
    const auto name = "native." + source.resolve_type.name +
                        "." + node->get_name();
    if (!irc.used_basic_convert_method.count(source.resolve_type.name)) {
        irc.used_basic_convert_method.insert({
            source.resolve_type.name,
            {}
        });
    }
    irc.used_basic_convert_method.at(source.resolve_type.name).insert(
        bsc->static_method.at(node->get_name()).return_type.name
    );
    auto result = value {
        .kind = value_kind::v_sfn,
        .resolve_type = node->get_resolve(),
        .content = name
    };
    value_stack.push_back(result);
}

bool generator::visit_call_path(call_path* node) {
    const auto source = value_stack.back();
    value_stack.pop_back();
    // basic symbol, for example i8, i16, i32, i64
    if (source.resolve_type.loc_file.empty()) {
        generate_basic_static_method(node, source);
        return true;
    }
    if (!ctx.global.domain.count(source.resolve_type.loc_file)) {
        err.err("code", node->get_location(),
            "cannot find domain \"" + source.resolve_type.loc_file + "\"."
        );
        return true;
    }
    const auto& dom = ctx.global.domain.at(source.resolve_type.loc_file);
    if (!dom.structs.count(source.resolve_type.name)) {
        err.err("code", node->get_location(),
            "cannot find struct \"" + source.resolve_type.name + "\"."
        );
        return true;
    }
    const auto& st = dom.structs.at(source.resolve_type.name);
    if (st.static_method.count(node->get_name())) {
        auto result = value {
            .kind = value_kind::v_sfn,
            .resolve_type = node->get_resolve(),
            .content = st.name + "." + node->get_name()
        };
        value_stack.push_back(result);
        return true;
    }
    unreachable(node);
    return true;
}

bool generator::visit_call_func_args(call_func_args* node) {
    std::vector<value> arguments;
    // load self if encountering value that is not a function
    if (value_stack.back().kind!=value_kind::v_fn &&
        value_stack.back().kind!=value_kind::v_sfn) {
        arguments.push_back(value_stack.back());
        value_stack.pop_back();
    }
    // load other arguments
    for(auto i : node->get_args()) {
        i->accept(this);
        arguments.push_back(value_stack.back());
        value_stack.pop_back();
    }

    if (value_stack.empty()) {
        report_stack_empty(node);
        return true;
    }
    const auto temp_0 = get_temp_variable();
    // generate function call ir
    const auto func = value_stack.back();
    value_stack.pop_back();
    auto new_call = new sir_call_func(
        func.content,
        type_convert(node->get_resolve()),
        node->get_resolve()==type::void_type()? "":temp_0
    );
    for(const auto& arg : arguments) {
        new_call->add_arg_type(type_convert(arg.resolve_type));
        new_call->add_arg(arg.content);
    }
    ircode_block->add_stmt(new_call);
    // push return value on stack
    if (node->get_resolve()!=type::void_type()) {
        const auto temp_1 = get_temp_variable();
        ircode_block->add_alloca(new sir_alloca(
            "real." + temp_1,
            type_convert(node->get_resolve())
        ));
        ircode_block->add_stmt(new sir_temp_ptr(
            temp_1,
            type_convert(node->get_resolve())
        ));
        ircode_block->add_stmt(new sir_store(
            type_convert(node->get_resolve()),
            temp_0,
            temp_1
        ));
        auto result = value {
            .kind = value_kind::v_var,
            .resolve_type = node->get_resolve(),
            .content = temp_1
        };
        result.resolve_type.pointer_depth++;
        value_stack.push_back(result);
    } else {
        value_stack.push_back({
            .kind = value_kind::v_var,
            .resolve_type = type::void_type(),
            .content = ""
        });
        ssa_temp_counter--;
    }
    return true;
}

bool generator::visit_definition(definition* node) {
    ircode_block->add_nop("begin def");
    node->get_init_value()->accept(this);
    if (node->get_type()) {
        ircode_block->add_alloca(new sir_alloca(
            node->get_name(),
            generate_type_string(node->get_type())
        ));
    } else {
        ircode_block->add_alloca(new sir_alloca(
            node->get_name(),
            type_convert(node->get_resolve())
        ));
    }
    const auto source = value_stack.back();
    value_stack.pop_back();
    if (node->get_type()) {
        ircode_block->add_stmt(new sir_store(
            generate_type_string(node->get_type()),
            source.content,
            node->get_name()
        ));
    } else {
        ircode_block->add_stmt(new sir_store(
            type_convert(node->get_resolve()),
            source.content,
            node->get_name()
        ));
    }
    ircode_block->add_nop("end def");
    return true;
}

void generator::generate_add_assignment(const value& left, const value& right) {
    auto left_type_copy = left.resolve_type;
    left_type_copy.pointer_depth--;
    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();
    ircode_block->add_stmt(new sir_load(
        type_convert(left_type_copy),
        left.content,
        temp_0
    ));
    ircode_block->add_stmt(new sir_add(
        temp_0, right.content, temp_1,
        left.resolve_type.is_integer(),
        type_convert(left_type_copy)
    ));
    ircode_block->add_stmt(new sir_store(
        type_convert(left_type_copy),
        temp_1,
        left.content
    ));
}

void generator::generate_sub_assignment(const value& left, const value& right) {
    auto left_type_copy = left.resolve_type;
    left_type_copy.pointer_depth--;
    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();
    ircode_block->add_stmt(new sir_load(
        type_convert(left_type_copy),
        left.content,
        temp_0
    ));
    ircode_block->add_stmt(new sir_sub(
        temp_0, right.content, temp_1,
        left.resolve_type.is_integer(),
        type_convert(left_type_copy)
    ));
    ircode_block->add_stmt(new sir_store(
        type_convert(left_type_copy),
        temp_1,
        left.content
    ));
}

void generator::generate_mul_assignment(const value& left, const value& right) {
    auto left_type_copy = left.resolve_type;
    left_type_copy.pointer_depth--;
    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();
    ircode_block->add_stmt(new sir_load(
        type_convert(left_type_copy),
        left.content,
        temp_0
    ));
    ircode_block->add_stmt(new sir_mul(
        temp_0, right.content, temp_1,
        left.resolve_type.is_integer(),
        type_convert(left_type_copy)
    ));
    ircode_block->add_stmt(new sir_store(
        type_convert(left_type_copy),
        temp_1,
        left.content
    ));
}

void generator::generate_div_assignment(const value& left, const value& right) {
    auto left_type_copy = left.resolve_type;
    left_type_copy.pointer_depth--;
    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();
    ircode_block->add_stmt(new sir_load(
        type_convert(left_type_copy),
        left.content,
        temp_0
    ));
    ircode_block->add_stmt(new sir_div(
        temp_0, right.content, temp_1,
        left.resolve_type.is_integer(),
        !left.resolve_type.is_unsigned(),
        type_convert(left_type_copy)
    ));
    ircode_block->add_stmt(new sir_store(
        type_convert(left_type_copy),
        temp_1,
        left.content
    ));
}

void generator::generate_rem_assignment(const value& left, const value& right) {
    auto left_type_copy = left.resolve_type;
    left_type_copy.pointer_depth--;
    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();
    ircode_block->add_stmt(new sir_load(
        type_convert(left_type_copy),
        left.content,
        temp_0
    ));
    ircode_block->add_stmt(new sir_rem(
        temp_0, right.content, temp_1,
        left.resolve_type.is_integer(),
        !left.resolve_type.is_unsigned(),
        type_convert(left_type_copy)
    ));
    ircode_block->add_stmt(new sir_store(
        type_convert(left_type_copy),
        temp_1,
        left.content
    ));
}

void generator::generate_eq_assignment(const value& left, const value& right) {
    ircode_block->add_stmt(new sir_store(
        type_convert(right.resolve_type),
        right.content,
        left.content
    ));
}

void generator::generate_and_assignment(const value& left, const value& right) {
    auto left_type_copy = left.resolve_type;
    left_type_copy.pointer_depth--;
    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();
    ircode_block->add_stmt(new sir_load(
        type_convert(left_type_copy),
        left.content,
        temp_0
    ));
    ircode_block->add_stmt(new sir_band(
        temp_0, right.content, temp_1,
        type_convert(left_type_copy)
    ));
    ircode_block->add_stmt(new sir_store(
        type_convert(left_type_copy),
        temp_1,
        left.content
    ));
}

void generator::generate_xor_assignment(const value& left, const value& right) {
    auto left_type_copy = left.resolve_type;
    left_type_copy.pointer_depth--;
    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();
    ircode_block->add_stmt(new sir_load(
        type_convert(left_type_copy),
        left.content,
        temp_0
    ));
    ircode_block->add_stmt(new sir_bxor(
        temp_0, right.content, temp_1,
        type_convert(left_type_copy)
    ));
    ircode_block->add_stmt(new sir_store(
        type_convert(left_type_copy),
        temp_1,
        left.content
    ));
}

void generator::generate_or_assignment(const value& left, const value& right) {
    auto left_type_copy = left.resolve_type;
    left_type_copy.pointer_depth--;
    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();
    ircode_block->add_stmt(new sir_load(
        type_convert(left_type_copy),
        left.content,
        temp_0
    ));
    ircode_block->add_stmt(new sir_bor(
        temp_0, right.content, temp_1,
        type_convert(left_type_copy)
    ));
    ircode_block->add_stmt(new sir_store(
        type_convert(left_type_copy),
        temp_1,
        left.content
    ));
}

bool generator::visit_assignment(assignment* node) {
    ircode_block->add_nop("begin assign");
    call_expression_generation(node->get_left(), true);
    const auto left = value_stack.back();
    value_stack.pop_back();

    node->get_right()->accept(this);
    const auto right = value_stack.back();
    value_stack.pop_back();

    switch(node->get_type()) {
        case assignment::kind::addeq: generate_add_assignment(left, right); break;
        case assignment::kind::diveq: generate_div_assignment(left, right); break;
        case assignment::kind::remeq: generate_rem_assignment(left, right); break;
        case assignment::kind::multeq: generate_mul_assignment(left, right); break;
        case assignment::kind::subeq: generate_sub_assignment(left, right); break;
        case assignment::kind::eq: generate_eq_assignment(left, right); break;
        case assignment::kind::andeq: generate_and_assignment(left, right); break;
        case assignment::kind::xoreq: generate_xor_assignment(left, right); break;
        case assignment::kind::oreq: generate_or_assignment(left, right); break;
        default: unimplemented(node);
    }
    ircode_block->add_nop("end assign");
    return true;
}

void generator::generate_and_operator(binary_operator* node) {
    ircode_block->add_nop("begin and opr");
    const auto temp_0 = get_temp_variable();
    ircode_block->add_alloca(new sir_alloca("real." + temp_0, "i1"));
    ircode_block->add_stmt(new sir_temp_ptr(temp_0, "i1"));
    node->get_left()->accept(this);
    const auto left = value_stack.back();
    value_stack.pop_back();
    ircode_block->add_stmt(new sir_store("i1", left.content, temp_0));
    auto br = new sir_br_cond(
        left.content,
        ircode_block->stmt_size()+1,
        0
    );
    ircode_block->add_stmt(br);
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    node->get_right()->accept(this);
    const auto right = value_stack.back();
    value_stack.pop_back();
    ircode_block->add_stmt(new sir_store("i1", right.content, temp_0));
    ircode_block->add_stmt(new sir_br(ircode_block->stmt_size()+1));
    br->set_false_label(ircode_block->stmt_size());
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    const auto temp_1 = get_temp_variable();
    ircode_block->add_stmt(new sir_load("i1", temp_0, temp_1));
    ircode_block->add_nop("end and opr");
    value_stack.push_back(value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = temp_1
    });
}

void generator::generate_or_operator(binary_operator* node) {
    ircode_block->add_nop("begin or opr");
    const auto temp_0 = get_temp_variable();
    ircode_block->add_alloca(new sir_alloca("real." + temp_0, "i1"));
    ircode_block->add_stmt(new sir_temp_ptr(temp_0, "i1"));
    node->get_left()->accept(this);
    const auto left = value_stack.back();
    value_stack.pop_back();
    ircode_block->add_stmt(new sir_store("i1", left.content, temp_0));
    auto br = new sir_br_cond(
        left.content,
        0,
        ircode_block->stmt_size()+1
    );
    ircode_block->add_stmt(br);
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    node->get_right()->accept(this);
    const auto right = value_stack.back();
    value_stack.pop_back();
    ircode_block->add_stmt(new sir_store("i1", right.content, temp_0));
    ircode_block->add_stmt(new sir_br(ircode_block->stmt_size()+1));
    br->set_true_label(ircode_block->stmt_size());
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    const auto temp_1 = get_temp_variable();
    ircode_block->add_stmt(new sir_load("i1", temp_0, temp_1));
    ircode_block->add_nop("end or opr");
    value_stack.push_back(value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = temp_1
    });
}

void generator::generate_add_operator(const value& left,
                                      const value& right,
                                      const value& result) {
    ircode_block->add_stmt(new sir_add(
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        type_convert(left.resolve_type)
    ));
}

void generator::generate_sub_operator(const value& left,
                                      const value& right,
                                      const value& result) {
    ircode_block->add_stmt(new sir_sub(
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        type_convert(left.resolve_type)
    ));
}

void generator::generate_mul_operator(const value& left,
                                      const value& right,
                                      const value& result) {
    ircode_block->add_stmt(new sir_mul(
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        type_convert(left.resolve_type)
    ));
}

void generator::generate_div_operator(const value& left,
                                      const value& right,
                                      const value& result) {
    ircode_block->add_stmt(new sir_div(
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        !left.resolve_type.is_unsigned(),
        type_convert(left.resolve_type)
    ));
}

void generator::generate_rem_operator(const value& left,
                                      const value& right,
                                      const value& result) {
    ircode_block->add_stmt(new sir_rem(
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        !left.resolve_type.is_unsigned(),
        type_convert(left.resolve_type)
    ));
}

void generator::generate_band_operator(const value& left,
                                       const value& right,
                                       const value& result) {
    ircode_block->add_stmt(new sir_band(
        left.content,
        right.content,
        result.content,
        type_convert(left.resolve_type)
    ));
}

void generator::generate_bxor_operator(const value& left,
                                       const value& right,
                                       const value& result) {
    ircode_block->add_stmt(new sir_bxor(
        left.content,
        right.content,
        result.content,
        type_convert(left.resolve_type)
    ));
}

void generator::generate_bor_operator(const value& left,
                                      const value& right,
                                      const value& result) {
    ircode_block->add_stmt(new sir_bor(
        left.content,
        right.content,
        result.content,
        type_convert(left.resolve_type)
    ));
}

void generator::generate_eq_operator(const value& left,
                                     const value& right,
                                     const value& result) {
    ircode_block->add_stmt(new sir_cmp(
        sir_cmp::kind::cmp_eq,
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        !left.resolve_type.is_unsigned(),
        type_convert(left.resolve_type)
    ));
}

void generator::generate_neq_operator(const value& left,
                                      const value& right,
                                      const value& result) {
    ircode_block->add_stmt(new sir_cmp(
        sir_cmp::kind::cmp_neq,
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        !left.resolve_type.is_unsigned(),
        type_convert(left.resolve_type)
    ));
}

void generator::generate_ge_operator(const value& left,
                                     const value& right,
                                     const value& result) {
    ircode_block->add_stmt(new sir_cmp(
        sir_cmp::kind::cmp_ge,
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        !left.resolve_type.is_unsigned(),
        type_convert(left.resolve_type)
    ));
}

void generator::generate_gt_operator(const value& left,
                                     const value& right,
                                     const value& result) {
    ircode_block->add_stmt(new sir_cmp(
        sir_cmp::kind::cmp_gt,
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        !left.resolve_type.is_unsigned(),
        type_convert(left.resolve_type)
    ));
}

void generator::generate_le_operator(const value& left,
                                     const value& right,
                                     const value& result) {
    ircode_block->add_stmt(new sir_cmp(
        sir_cmp::kind::cmp_le,
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        !left.resolve_type.is_unsigned(),
        type_convert(left.resolve_type)
    ));
}

void generator::generate_lt_operator(const value& left,
                                     const value& right,
                                     const value& result) {
    ircode_block->add_stmt(new sir_cmp(
        sir_cmp::kind::cmp_lt,
        left.content,
        right.content,
        result.content,
        left.resolve_type.is_integer(),
        !left.resolve_type.is_unsigned(),
        type_convert(left.resolve_type)
    ));
}

bool generator::visit_binary_operator(binary_operator* node) {
    if (node->get_opr()==binary_operator::kind::cmpand) {
        generate_and_operator(node);
        return true;
    }
    if (node->get_opr()==binary_operator::kind::cmpor) {
        generate_or_operator(node);
        return true;
    }
    ircode_block->add_nop("begin binary opr");
    node->get_left()->accept(this);
    const auto left = value_stack.back();
    value_stack.pop_back();
    node->get_right()->accept(this);
    const auto right = value_stack.back();
    value_stack.pop_back();
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    value_stack.push_back(result);
    switch(node->get_opr()) {
        case binary_operator::kind::add: generate_add_operator(left, right, result); break;
        case binary_operator::kind::sub: generate_sub_operator(left, right, result); break;
        case binary_operator::kind::mult: generate_mul_operator(left, right, result); break;
        case binary_operator::kind::div: generate_div_operator(left, right, result); break;
        case binary_operator::kind::rem: generate_rem_operator(left, right, result); break;
        case binary_operator::kind::band: generate_band_operator(left, right, result); break;
        case binary_operator::kind::bxor: generate_bxor_operator(left, right, result); break;
        case binary_operator::kind::bor: generate_bor_operator(left, right, result); break;
        case binary_operator::kind::cmpeq: generate_eq_operator(left, right, result); break;
        case binary_operator::kind::cmpneq: generate_neq_operator(left, right, result); break;
        case binary_operator::kind::geq: generate_ge_operator(left, right, result); break;
        case binary_operator::kind::grt: generate_gt_operator(left, right, result); break;
        case binary_operator::kind::leq: generate_le_operator(left, right, result); break;
        case binary_operator::kind::less: generate_lt_operator(left, right, result); break;
        default: unimplemented(node);
    }
    ircode_block->add_nop("end binary opr");
    return true;
}

void generator::generate_neg_operator(const value& src, const value& result) {
    ircode_block->add_stmt(new sir_neg(
        src.content,
        result.content,
        src.resolve_type.is_integer(),
        type_convert(src.resolve_type)
    ));
}

void generator::generate_bnot_operator(const value& src, const value& result) {
    ircode_block->add_stmt(new sir_bnot(
        src.content,
        result.content,
        type_convert(src.resolve_type)
    ));
}

bool generator::visit_unary_operator(unary_operator* node) {
    ircode_block->add_nop("begin unary opr");
    node->get_value()->accept(this);
    const auto src = value_stack.back();
    value_stack.pop_back();
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    value_stack.push_back(result);
    switch(node->get_opr()) {
        case unary_operator::kind::neg: generate_neg_operator(src,result); break;
        case unary_operator::kind::bnot: generate_bnot_operator(src, result); break;
        default: unimplemented(node);
    }
    ircode_block->add_nop("end binary opr");
    return true;
}

bool generator::visit_ret_stmt(ret_stmt* node) {
    if (node->get_value()) {
        node->get_value()->accept(this);
        const auto result = value_stack.back();
        value_stack.pop_back();
        ircode_block->add_stmt(new sir_ret(
            type_convert(result.resolve_type),
            result.content
        ));
    } else {
        ircode_block->add_stmt(new sir_ret("void"));
    }
    return true;
}

bool generator::visit_while_stmt(while_stmt* node) {
    ircode_block->add_nop("begin loop");
    auto while_begin_label = ircode_block->stmt_size()+1;
    loop_begin.push_back(while_begin_label);
    break_inst.clear();
    // condition
    ircode_block->add_stmt(new sir_br(ircode_block->stmt_size()+1));
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));

    node->get_condition()->accept(this);
    const auto condition = value_stack.back();
    value_stack.pop_back();
    auto cond_branch_ir = new sir_br_cond(
        condition.content,
        ircode_block->stmt_size()+1,
        0
    );
    ircode_block->add_stmt(cond_branch_ir);

    // loop begin
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    node->get_block()->accept(this);
    ircode_block->add_stmt(new sir_br(while_begin_label));

    // loop exit
    auto while_exit_label = ircode_block->stmt_size();
    cond_branch_ir->set_false_label(while_exit_label);
    ircode_block->add_stmt(new sir_label(while_exit_label));
    for(auto i : break_inst) {
        i->set_label(while_exit_label);
    }
    ircode_block->add_nop("end loop");
    loop_begin.pop_back();
    return true;
}

bool generator::visit_continue_stmt(continue_stmt* node) {
    ircode_block->add_stmt(new sir_br(loop_begin.back()));
    ircode_block->add_stmt(new sir_place_holder_label(get_place_holder_label()));
    return true;
}

bool generator::visit_break_stmt(break_stmt* node) {
    auto break_br = new sir_br(0);
    break_inst.push_back(break_br);
    ircode_block->add_stmt(break_br);
    ircode_block->add_stmt(new sir_place_holder_label(get_place_holder_label()));
    return true;
}

bool generator::visit_if_stmt(if_stmt* node) {
    sir_br_cond* br_cond = nullptr;
    if (node->get_condition()) {
        node->get_condition()->accept(this);
        const auto condition = value_stack.back();
        value_stack.pop_back();
        br_cond = new sir_br_cond(
            condition.content,
            ircode_block->stmt_size()+1,
            0
        );
        ircode_block->add_stmt(br_cond);
    }
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    node->get_block()->accept(this);
    if (ircode_block->back_is_ret_stmt()) {
        // generate a new label here
        ircode_block->add_stmt(new sir_place_holder_label(get_place_holder_label()));
    }
    auto jump_out = new sir_br(0);
    jump_outs.push_back(jump_out);
    ircode_block->add_stmt(jump_out);
    if (br_cond) {
        br_cond->set_false_label(ircode_block->stmt_size());
        ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    }
    return true;
}

bool generator::visit_cond_stmt(cond_stmt* node) {
    ircode_block->add_nop("begin cond");
    jump_outs.clear();
    for(auto i : node->get_stmts()) {
        if (!i->get_condition()) {
            // last branch: must be else branch
            ircode_block->add_stmt(new sir_br(ircode_block->stmt_size()+1));
        }
        i->accept(this);
        if (i->get_condition() && i==node->get_stmts().back()) {
            // elsif branch
            ircode_block->add_stmt(new sir_br(ircode_block->stmt_size()+1));
        }
    }
    for(auto i : jump_outs) {
        i->set_label(ircode_block->stmt_size());
    }
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    ircode_block->add_nop("end cond");
    return true;
}

}