#include "code/ir_gen.h"
#include "sema/symbol.h"
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

bool ir_gen::visit_func_decl(func_decl* node) {
    ssa_temp_counter = 1;
    auto name = node->get_name();
    if (impl_struct_name.length()) {
        name = impl_struct_name + "." + name;
    }
    auto new_ir = new hir_func("@" + name);
    new_ir->set_return_type(generate_type_string(node->get_return_type()));
    for(auto i : node->get_params()->get_params()) {
        new_ir->add_param(
            i->get_name()->get_name(),
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
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    ircode_block->add_allocs(new sir_alloca(
        result.content,
        type_convert(node->get_resolve())
    ));
    ircode_block->add_stmt(new sir_store(
        type_convert(node->get_resolve()),
        node->get_number(),
        result.content
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
    ircode_block->add_allocs(new sir_alloca(
        result.content,
        "i1"
    ));
    ircode_block->add_stmt(new sir_store(
        "i1",
        node->get_flag()? "true":"false",
        result.content
    ));
    value_stack.push_back(result);
    return true;
}

bool ir_gen::visit_call(call* node) {
    // call head may be the global function
    if (ctx.global_symbol.count(node->get_head()->get_name()) &&
        ctx.global_symbol.at(node->get_head()->get_name()).kind==symbol_kind::func_kind) {
        value_stack.push_back({
            .kind = value_kind::v_sfn,
            .resolve_type = node->get_head()->get_resolve(),
            .content = node->get_head()->get_name()
        });
    } else {
        value_stack.push_back({
            .kind = value_kind::v_var,
            .resolve_type = node->get_head()->get_resolve(),
            .content = node->get_head()->get_name()
        });
    }
    for(auto i : node->get_chain()) {
        i->accept(this);
    }
    const auto source = value_stack.back();
    value_stack.pop_back();
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    ircode_block->add_stmt(new sir_store(
        type_convert(result.resolve_type),
        source.content,
        result.content
    ));
    value_stack.push_back(result);
    return true;
}

bool ir_gen::visit_call_index(call_index* node) {
    node->get_index()->accept(this);
    const auto index = value_stack.back();
    value_stack.pop_back();
    const auto source = value_stack.back();
    value_stack.pop_back();
    auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    result.resolve_type.pointer_level--;
    value_stack.push_back(result);
    ircode_block->add_stmt(new sir_call_index(
        source.content,
        result.content,
        index.content
    ));
    return true;
}

bool ir_gen::visit_call_field(call_field* node) {
    ircode_block->add_stmt(new sir_call_field(node->get_name()));
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
        auto result = value {
            .kind = value_kind::v_var,
            .resolve_type = node->get_resolve(),
            .content = get_temp_variable()
        };
        value_stack.push_back(result);
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
    ircode_block->add_stmt(new sir_ptr_call_field(node->get_name()));
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
        auto result = value {
            .kind = value_kind::v_var,
            .resolve_type = node->get_resolve(),
            .content = get_temp_variable()
        };
        value_stack.push_back(result);
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
    for(auto i : node->get_args()) {
        i->accept(this);
        arguments.push_back(value_stack.back());
        value_stack.pop_back();
    }
    if (value_stack.empty()) {
        report_stack_empty(node);
        return true;
    }

    const auto func = value_stack.back();
    value_stack.pop_back();
    auto new_call = new sir_call_func(func.content);
    for(const auto& arg : arguments) {
        new_call->add_arg(arg.content);
    }
    ircode_block->add_stmt(new_call);
    const auto result = value {
        .kind = value_kind::v_var,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    value_stack.push_back(result);
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
    return true;
}

bool ir_gen::visit_assignment(assignment* node) {
    node->get_left()->accept(this);
    node->get_right()->accept(this);
    switch(node->get_type()) {
        case assignment::kind::addeq: ircode_block->add_stmt(new sir_assign("+=")); break;
        case assignment::kind::diveq: ircode_block->add_stmt(new sir_assign("/=")); break;
        case assignment::kind::eq: ircode_block->add_stmt(new sir_assign("=")); break;
        case assignment::kind::modeq: ircode_block->add_stmt(new sir_assign("%=")); break;
        case assignment::kind::multeq: ircode_block->add_stmt(new sir_assign("*=")); break;
        case assignment::kind::subeq: ircode_block->add_stmt(new sir_assign("-=")); break;
    }
    return true;
}

bool ir_gen::visit_binary_operator(binary_operator* node) {
    if (node->get_opr()==binary_operator::kind::cmpand) {
        node->get_left()->accept(this);
        auto br = new sir_br_cond(ircode_block->stmt_size()+1, 0);
        ircode_block->add_stmt(br);
        ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
        node->get_right()->accept(this);
        br->set_false_label(ircode_block->stmt_size());
        ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
        return true;
    }
    if (node->get_opr()==binary_operator::kind::cmpor) {
        node->get_left()->accept(this);
        auto br = new sir_br_cond(0, ircode_block->stmt_size()+1);
        ircode_block->add_stmt(br);
        ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
        node->get_right()->accept(this);
        br->set_true_label(ircode_block->stmt_size());
        ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
        return true;
    }
    node->get_left()->accept(this);
    node->get_right()->accept(this);
    switch(node->get_opr()) {
        case binary_operator::kind::add: ircode_block->add_stmt(new sir_binary("+")); break;
        case binary_operator::kind::cmpand: ircode_block->add_stmt(new sir_binary("&&")); break;
        case binary_operator::kind::cmpeq: ircode_block->add_stmt(new sir_binary("==")); break;
        case binary_operator::kind::cmpneq: ircode_block->add_stmt(new sir_binary("!=")); break;
        case binary_operator::kind::cmpor: ircode_block->add_stmt(new sir_binary("||")); break;
        case binary_operator::kind::div: ircode_block->add_stmt(new sir_binary("/")); break;
        case binary_operator::kind::geq: ircode_block->add_stmt(new sir_binary(">=")); break;
        case binary_operator::kind::grt: ircode_block->add_stmt(new sir_binary(">")); break;
        case binary_operator::kind::leq: ircode_block->add_stmt(new sir_binary("<=")); break;
        case binary_operator::kind::less: ircode_block->add_stmt(new sir_binary("<")); break;
        case binary_operator::kind::mod: ircode_block->add_stmt(new sir_binary("%")); break;
        case binary_operator::kind::mult: ircode_block->add_stmt(new sir_binary("*")); break;
        case binary_operator::kind::sub: ircode_block->add_stmt(new sir_binary("-")); break;
    }
    return true;
}

bool ir_gen::visit_ret_stmt(ret_stmt* node) {
    if (node->get_value()) {
        node->get_value()->accept(this);
    }
    ircode_block->add_stmt(new sir_ret(node->get_value()));
    return true;
}

bool ir_gen::visit_while_stmt(while_stmt* node) {
    auto while_begin_label = ircode_block->stmt_size();
    // condition
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));

    node->get_condition()->accept(this);
    auto cond_branch_ir = new sir_br_cond(ircode_block->stmt_size()+1, 0);
    ircode_block->add_stmt(cond_branch_ir);

    // loop begin
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    node->get_block()->accept(this);
    ircode_block->add_stmt(new sir_br(while_begin_label));

    // loop exit
    cond_branch_ir->set_false_label(ircode_block->stmt_size());
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    return true;
}

bool ir_gen::visit_if_stmt(if_stmt* node) {
    sir_br_cond* br_cond = nullptr;
    if (node->get_condition()) {
        node->get_condition()->accept(this);
        br_cond = new sir_br_cond(ircode_block->stmt_size()+1, 0);
        ircode_block->add_stmt(br_cond);
    }
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    node->get_block()->accept(this);
    if (br_cond) {
        br_cond->set_false_label(ircode_block->stmt_size());
    }
    ircode_block->add_stmt(new sir_label(ircode_block->stmt_size()));
    return true;
}

}