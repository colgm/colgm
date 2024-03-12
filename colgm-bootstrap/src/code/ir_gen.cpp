#include "code/ir_gen.h"
#include "sema/symbol.h"
#include "semantic.h"

namespace colgm {

std::string ir_gen::generate_type_string(type_def* node) {
    auto ty = type({
        node->get_name()->get_name(),
        node->get_location().file,
        node->get_pointer_level()
    });
    if (ctx.global_symbol.count(ty.name) &&
        ctx.global_symbol.at(ty.name).kind==symbol_kind::struct_kind) {
        ty.name = "%struct." + ty.name;
    } else if (basic_type_convert_mapper.count(ty.name)) {
        ty.name = basic_type_convert_mapper.at(ty.name);
    }
    return ty.to_string();
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
        new_ir->set_code_block(new sir_code_block);
    } else {
        emit_func_decl(new_ir);
        return true;
    }
    cb = new_ir->get_code_block();
    node->get_code_block()->accept(this);
    cb = nullptr;
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
        .kind = value_kind::v_id,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    cb->add_stmt(new sir_number(node->get_number(), result.content));
    value_stack.push_back(result);
    return true;
}

bool ir_gen::visit_string_literal(string_literal* node) {
    const auto result = value {
        .kind = value_kind::v_id,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    cb->add_stmt(new sir_string(node->get_string(), result.content));
    irs.const_strings.insert({node->get_string(), irs.const_strings.size()});
    value_stack.push_back(result);
    return true;
}

bool ir_gen::visit_bool_literal(bool_literal* node) {
    const auto result = value {
        .kind = value_kind::v_id,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    cb->add_stmt(new sir_bool(node->get_flag(), result.content));
    value_stack.push_back(result);
    return true;
}

bool ir_gen::visit_call(call* node) {
    cb->add_stmt(new sir_get_var(node->get_head()->get_name()));
    value_stack.push_back({
        .kind = value_kind::v_id,
        .resolve_type = node->get_head()->get_resolve(),
        .content = node->get_head()->get_name()
    });
    for(auto i : node->get_chain()) {
        i->accept(this);
    }
    return true;
}

bool ir_gen::visit_call_index(call_index* node) {
    node->get_index()->accept(this);
    const auto index = value_stack.back();
    value_stack.pop_back();
    const auto source = value_stack.back();
    value_stack.pop_back();
    const auto result = value {
        .kind = value_kind::v_id,
        .resolve_type = node->get_resolve(),
        .content = get_temp_variable()
    };
    value_stack.push_back(result);
    cb->add_stmt(new sir_call_index(
        source.content,
        result.content,
        index.content
    ));
    return true;
}

bool ir_gen::visit_call_field(call_field* node) {
    cb->add_stmt(new sir_call_field(node->get_name()));
    return true;
}

bool ir_gen::visit_ptr_call_field(ptr_call_field* node) {
    cb->add_stmt(new sir_ptr_call_field(node->get_name()));
    return true;
}

bool ir_gen::visit_call_path(call_path* node) {
    cb->add_stmt(new sir_call_path(node->get_name()));
    return true;
}

bool ir_gen::visit_call_func_args(call_func_args* node) {
    for(auto i : node->get_args()) {
        i->accept(this);
    }
    cb->add_stmt(new sir_call_func(node->get_args().size()));
    return true;
}

bool ir_gen::visit_definition(definition* node) {
    node->get_init_value()->accept(this);
    cb->add_stmt(new sir_alloca(
        node->get_name(),
        generate_type_string(node->get_type())
    ));
    return true;
}

bool ir_gen::visit_assignment(assignment* node) {
    node->get_left()->accept(this);
    node->get_right()->accept(this);
    switch(node->get_type()) {
        case assignment::kind::addeq: cb->add_stmt(new sir_assign("+=")); break;
        case assignment::kind::diveq: cb->add_stmt(new sir_assign("/=")); break;
        case assignment::kind::eq: cb->add_stmt(new sir_assign("=")); break;
        case assignment::kind::modeq: cb->add_stmt(new sir_assign("%=")); break;
        case assignment::kind::multeq: cb->add_stmt(new sir_assign("*=")); break;
        case assignment::kind::subeq: cb->add_stmt(new sir_assign("-=")); break;
    }
    return true;
}

bool ir_gen::visit_binary_operator(binary_operator* node) {
    if (node->get_opr()==binary_operator::kind::cmpand) {
        node->get_left()->accept(this);
        auto br = new sir_br_cond(cb->size()+1, 0);
        cb->add_stmt(br);
        cb->add_stmt(new sir_label(cb->size()));
        node->get_right()->accept(this);
        br->set_false_label(cb->size());
        cb->add_stmt(new sir_label(cb->size()));
        return true;
    }
    if (node->get_opr()==binary_operator::kind::cmpor) {
        node->get_left()->accept(this);
        auto br = new sir_br_cond(0, cb->size()+1);
        cb->add_stmt(br);
        cb->add_stmt(new sir_label(cb->size()));
        node->get_right()->accept(this);
        br->set_true_label(cb->size());
        cb->add_stmt(new sir_label(cb->size()));
        return true;
    }
    node->get_left()->accept(this);
    node->get_right()->accept(this);
    switch(node->get_opr()) {
        case binary_operator::kind::add: cb->add_stmt(new sir_binary("+")); break;
        case binary_operator::kind::cmpand: cb->add_stmt(new sir_binary("&&")); break;
        case binary_operator::kind::cmpeq: cb->add_stmt(new sir_binary("==")); break;
        case binary_operator::kind::cmpneq: cb->add_stmt(new sir_binary("!=")); break;
        case binary_operator::kind::cmpor: cb->add_stmt(new sir_binary("||")); break;
        case binary_operator::kind::div: cb->add_stmt(new sir_binary("/")); break;
        case binary_operator::kind::geq: cb->add_stmt(new sir_binary(">=")); break;
        case binary_operator::kind::grt: cb->add_stmt(new sir_binary(">")); break;
        case binary_operator::kind::leq: cb->add_stmt(new sir_binary("<=")); break;
        case binary_operator::kind::less: cb->add_stmt(new sir_binary("<")); break;
        case binary_operator::kind::mod: cb->add_stmt(new sir_binary("%")); break;
        case binary_operator::kind::mult: cb->add_stmt(new sir_binary("*")); break;
        case binary_operator::kind::sub: cb->add_stmt(new sir_binary("-")); break;
    }
    return true;
}

bool ir_gen::visit_ret_stmt(ret_stmt* node) {
    if (node->get_value()) {
        node->get_value()->accept(this);
    }
    cb->add_stmt(new sir_ret(node->get_value()));
    return true;
}

bool ir_gen::visit_while_stmt(while_stmt* node) {
    auto while_begin_label = cb->size();
    // condition
    cb->add_stmt(new sir_label(cb->size()));

    node->get_condition()->accept(this);
    auto cond_branch_ir = new sir_br_cond(cb->size()+1, 0);
    cb->add_stmt(cond_branch_ir);

    // loop begin
    cb->add_stmt(new sir_label(cb->size()));
    node->get_block()->accept(this);
    cb->add_stmt(new sir_br_direct(while_begin_label));

    // loop exit
    cond_branch_ir->set_false_label(cb->size());
    cb->add_stmt(new sir_label(cb->size()));
    return true;
}

bool ir_gen::visit_if_stmt(if_stmt* node) {
    sir_br_cond* br_cond = nullptr;
    if (node->get_condition()) {
        node->get_condition()->accept(this);
        br_cond = new sir_br_cond(cb->size()+1, 0);
        cb->add_stmt(br_cond);
    }
    cb->add_stmt(new sir_label(cb->size()));
    node->get_block()->accept(this);
    if (br_cond) {
        br_cond->set_false_label(cb->size());
    }
    cb->add_stmt(new sir_label(cb->size()));
    return true;
}

}