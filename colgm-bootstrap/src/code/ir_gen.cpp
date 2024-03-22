#include "code/ir_gen.h"
#include "sema/symbol.h"
#include "sema/basic.h"
#include "semantic.h"

namespace colgm {

std::string ir_gen::type_convert(const type& t) {
    auto copy = t;
    if (basic_type_convert_mapper.count(copy.name)) {
        copy.name = basic_type_convert_mapper.at(copy.name);
    } else if (ctx.global_symbol.count(copy.name) &&
        ctx.global_symbol.at(copy.name).kind==symbol_kind::struct_kind) {
        copy.name = "%struct." + copy.name;
    }
    return copy.to_string();
}

std::string ir_gen::generate_type_string(type_def* node) {
    return type_convert({
        node->get_name()->get_name(),
        node->get_location().file,
        node->get_pointer_level()
    });
}

bool ir_gen::visit_struct_decl(struct_decl* node) {
    auto new_struct = hir_struct(node->get_name());
    for(auto i : node->get_fields()) {
        new_struct.add_field_type(generate_type_string(i->get_type()));
    }
    emit_struct_decl(new_struct);
    return true;
}

void ir_gen::convert_parameter_to_pointer(func_decl* node) {
    for(auto i : node->get_params()->get_params()) {
        auto param_addr = new sir_alloca(
            i->get_name()->get_name(),
            generate_type_string(i->get_type())
        );
        ircode_block->add_stmt(param_addr);
        auto store_param_to_addr = new sir_store(
            generate_type_string(i->get_type()),
            i->get_name()->get_name() + ".param",
            i->get_name()->get_name()
        );
        ircode_block->add_stmt(store_param_to_addr);
    }
}

bool ir_gen::visit_func_decl(func_decl* node) {
    ssa_temp_counter = 1;
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

bool ir_gen::visit_impl_struct(impl_struct* node) {
    impl_struct_name = node->get_struct_name();
    for(auto i : node->get_methods()) {
        i->accept(this);
    }
    impl_struct_name = "";
    return true;
}

bool ir_gen::visit_number_literal(number_literal* node) {
    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = temp_1
    };
    ircode_block->add_stmt(new sir_alloca(
        temp_0,
        type_convert(node->get_resolve())
    ));
    ircode_block->add_stmt(new sir_store_literal(
        type_convert(node->get_resolve()),
        node->get_number(),
        temp_0
    ));
    ircode_block->add_stmt(new sir_load(
        type_convert(node->get_resolve()),
        temp_0,
        temp_1
    ));
    value_stack.push_back(result);
    return true;
}

bool ir_gen::visit_string_literal(string_literal* node) {
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

bool ir_gen::visit_bool_literal(bool_literal* node) {
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    ircode_block->add_stmt(new sir_alloca(
        result.content,
        "i1"
    ));
    ircode_block->add_stmt(new sir_store_literal(
        "i1",
        node->get_flag()? "true":"false",
        result.content
    ));
    value_stack.push_back(result);
    return true;
}

void ir_gen::call_function_symbol(identifier* node) {
    value_stack.push_back({
        .kind = value_kind::v_sfn,
        .resolve_type = node->get_resolve(),
        .content = node->get_name()
    });
}

void ir_gen::call_variable(identifier* node) {
    const auto temp_0 = get_temp_variable();
    ircode_block->add_stmt(new sir_load(
        type_convert(node->get_resolve()),
        node->get_name(),
        temp_0
    ));
    value_stack.push_back({
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = temp_0
    });
}

bool ir_gen::visit_call(call* node) {
    // call head may be the global function
    auto head = node->get_head();
    if (ctx.global_symbol.count(head->get_name()) &&
        ctx.global_symbol.at(head->get_name()).kind==symbol_kind::func_kind) {
        call_function_symbol(head);
    } else {
        call_variable(head);
    }
    for(auto i : node->get_chain()) {
        i->accept(this);
    }
    ircode_block->add_nop();
    return true;
}

bool ir_gen::visit_call_index(call_index* node) {
    node->get_index()->accept(this);
    const auto index = value_stack.back();
    value_stack.pop_back();
    const auto source = value_stack.back();
    value_stack.pop_back();
    const auto temp_0 = get_temp_variable();
    const auto temp_1 = get_temp_variable();
    auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = temp_1
    };
    value_stack.push_back(result);
    ircode_block->add_stmt(new sir_call_index(
        source.content,
        temp_0,
        index.content,
        type_convert(node->get_resolve()),
        type_convert(index.resolve_type)
    ));
    ircode_block->add_stmt(new sir_load(
        type_convert(node->get_resolve()),
        temp_0,
        temp_1
    ));
    return true;
}

bool ir_gen::visit_call_field(call_field* node) {
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
        const auto temp_0 = get_temp_variable();
        const auto temp_1 = get_temp_variable();
        const auto temp_2 = get_temp_variable();
        ircode_block->add_stmt(new sir_alloca(
            temp_0,
            type_convert(source.resolve_type)
        ));
        ircode_block->add_stmt(new sir_store(
            type_convert(source.resolve_type),
            source.content,
            temp_0
        ));
        auto result = value {
            .kind = value_kind::v_var,
            .resolve_type = node->get_resolve(),
            .content = temp_2
        };
        value_stack.push_back(result);
        for(usize i = 0; i<st.ordered_field.size(); ++i) {
            if (node->get_name()==st.ordered_field[i].name) {
                ircode_block->add_stmt(new sir_call_field(
                    temp_1,
                    temp_0,
                    type_convert(source.resolve_type),
                    i
                ));
                ircode_block->add_stmt(new sir_load(
                    type_convert(st.ordered_field[i].symbol_type),
                    temp_1,
                    temp_2
                ));
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

bool ir_gen::visit_ptr_call_field(ptr_call_field* node) {
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
        const auto temp_0 = get_temp_variable();
        const auto temp_1 = get_temp_variable();
        auto source_type_copy = source.resolve_type;
        source_type_copy.pointer_depth--;

        auto result = value {
            .kind = value_kind::v_var,
            .resolve_type = node->get_resolve(),
            .content = temp_1
        };
        value_stack.push_back(result);
        for(usize i = 0; i<st.ordered_field.size(); ++i) {
            if (node->get_name()==st.ordered_field[i].name) {
                ircode_block->add_stmt(new sir_call_field(
                    temp_0,
                    source.content,
                    type_convert(source_type_copy),
                    i
                ));
                ircode_block->add_stmt(new sir_load(
                    type_convert(st.ordered_field[i].symbol_type),
                    temp_0,
                    temp_1
                ));
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

bool ir_gen::visit_call_path(call_path* node) {
    const auto source = value_stack.back();
    value_stack.pop_back();
    if (source.resolve_type.loc_file.empty()) {
        const auto bsc = colgm_basic::mapper.at(source.resolve_type.name);
        if (bsc->static_method.count(node->get_name())) {
            auto result = value {
                .kind = value_kind::v_sfn,
                .resolve_type = node->get_resolve(),
                .content = "." + source.resolve_type.name + "." + node->get_name()
            };
            value_stack.push_back(result);
            return true;
        }
        unreachable(node);
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

bool ir_gen::visit_call_func_args(call_func_args* node) {
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
    // generate return value but do not push now
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    // generate function call ir
    const auto func = value_stack.back();
    value_stack.pop_back();
    auto new_call = new sir_call_func(
        func.content,
        type_convert(node->get_resolve()),
        node->get_resolve()==type::void_type()? "":result.content
    );
    for(const auto& arg : arguments) {
        new_call->add_arg_type(type_convert(arg.resolve_type));
        new_call->add_arg(arg.content);
    }
    ircode_block->add_stmt(new_call);
    // push return value on stack
    if (node->get_resolve()!=type::void_type()) {
        value_stack.push_back(result);
    } else {
        ssa_temp_counter--;
    }
    return true;
}

bool ir_gen::visit_definition(definition* node) {
    node->get_init_value()->accept(this);
    ircode_block->add_stmt(new sir_alloca(
        node->get_name(),
        generate_type_string(node->get_type())
    ));
    const auto source = value_stack.back();
    value_stack.pop_back();
    ircode_block->add_stmt(new sir_store(
        generate_type_string(node->get_type()),
        source.content,
        node->get_name()
    ));
    ircode_block->add_nop();
    return true;
}

bool ir_gen::visit_assignment(assignment* node) {
    node->get_left()->accept(this);
    const auto left = value_stack.back();
    value_stack.pop_back();

    node->get_right()->accept(this);
    const auto right = value_stack.back();
    value_stack.pop_back();

    switch(node->get_type()) {
        case assignment::kind::addeq:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, left.content, "add",
                type_convert(left.resolve_type)
            )); break;
        case assignment::kind::diveq:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, left.content, "div",
                type_convert(left.resolve_type)
            )); break;
        case assignment::kind::eq:
            ircode_block->add_stmt(new sir_store(
                type_convert(left.resolve_type),
                right.content,
                left.content
            )); break;
        case assignment::kind::modeq:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, left.content, "rem",
                type_convert(left.resolve_type)
            )); break;
        case assignment::kind::multeq:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, left.content, "mul",
                type_convert(left.resolve_type)
            )); break;
        case assignment::kind::subeq:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, left.content, "sub",
                type_convert(left.resolve_type)
            )); break;
    }
    ircode_block->add_nop();
    return true;
}

void ir_gen::generate_and_operator(binary_operator* node) {
    node->get_left()->accept(this);
    const auto condition = value_stack.back();
    value_stack.pop_back();
    auto br = new sir_br_cond(
        condition.content,
        ircode_block->stmt_size()+1,
        0
    );
    ircode_block->add_stmt(br);
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    node->get_right()->accept(this);
    br->set_false_label(ircode_block->stmt_size());
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    ircode_block->add_nop();
}

void ir_gen::generate_or_operator(binary_operator* node) {
    node->get_left()->accept(this);
    const auto condition = value_stack.back();
    value_stack.pop_back();
    auto br = new sir_br_cond(
        condition.content,
        0,
        ircode_block->stmt_size()+1
    );
    ircode_block->add_stmt(br);
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    node->get_right()->accept(this);
    br->set_true_label(ircode_block->stmt_size());
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    ircode_block->add_nop();
}

bool ir_gen::visit_binary_operator(binary_operator* node) {
    if (node->get_opr()==binary_operator::kind::cmpand) {
        generate_and_operator(node);
        return true;
    }
    if (node->get_opr()==binary_operator::kind::cmpor) {
        generate_or_operator(node);
        return true;
    }
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
        case binary_operator::kind::add:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "add",
                type_convert(node->get_resolve())
            )); break;
        case binary_operator::kind::cmpeq:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "eq",
                type_convert(node->get_resolve())
            )); break;
        case binary_operator::kind::cmpneq:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "ne",
                type_convert(node->get_resolve())
            )); break;
        case binary_operator::kind::div:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "div",
                type_convert(node->get_resolve())
            )); break;
        case binary_operator::kind::geq:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "ge",
                type_convert(node->get_resolve())
            )); break;
        case binary_operator::kind::grt:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "gt",
                type_convert(node->get_resolve())
            )); break;
        case binary_operator::kind::leq:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "le",
                type_convert(node->get_resolve())
            )); break;
        case binary_operator::kind::less:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "lt",
                type_convert(node->get_resolve())
            )); break;
        case binary_operator::kind::mod:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "rem",
                type_convert(node->get_resolve())
            )); break;
        case binary_operator::kind::mult:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "mul",
                type_convert(node->get_resolve())
            )); break;
        case binary_operator::kind::sub:
            ircode_block->add_stmt(new sir_binary(
                left.content, right.content, result.content, "sub",
                type_convert(node->get_resolve())
            )); break;
        default: unreachable(node);
    }
    ircode_block->add_nop();
    return true;
}

bool ir_gen::visit_ret_stmt(ret_stmt* node) {
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

bool ir_gen::visit_while_stmt(while_stmt* node) {
    auto while_begin_label = ircode_block->stmt_size();
    // condition
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
    cond_branch_ir->set_false_label(ircode_block->stmt_size());
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    ircode_block->add_nop();
    return true;
}

bool ir_gen::visit_if_stmt(if_stmt* node) {
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
    if (br_cond) {
        br_cond->set_false_label(ircode_block->stmt_size());
    }
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    ircode_block->add_nop();
    return true;
}

}