#include "mir/mir2sir.h"

#include <fstream>
#include <cassert>

namespace colgm::mir {

void mir2sir::generate_type_mapper() {
    type_mapper = {};
    for(const auto& dm : ctx.global.domain) {
        for(const auto& i : dm.second.enums) {
            const auto tp = type {
                .name = i.second.name,
                .loc_file = i.second.location.file
            };
            type_mapper.insert({tp.full_path_name(), sym_kind::enum_kind});
        }
        for(const auto& i : dm.second.structs) {
            const auto tp = type {
                .name = i.second.name,
                .loc_file = i.second.location.file
            };
            type_mapper.insert({tp.full_path_name(), sym_kind::struct_kind});
        }
    }
}

std::string mir2sir::type_mapping(const type& t) {
    auto copy = t;
    if (basic_type_mapper.count(copy.name)) {
        copy.name = basic_type_mapper.at(copy.name);
        return copy.to_string();
    }
    const auto full_name = t.full_path_name();
    if (!type_mapper.count(full_name)) {
        return copy.to_string();
    }
    switch(type_mapper.at(full_name)) {
        case sym_kind::struct_kind:
            copy.name = "%struct." + mangle(full_name);
            break;
        case sym_kind::enum_kind: copy = type::i64_type(); break;
        default: break;
    }
    return copy.to_string();
}

void mir2sir::emit_struct(const mir_context& mctx) {
    for(auto i : mctx.structs) {
        auto stct = new sir_struct(i->name, i->location);
        for(const auto& f : i->field_type) {
            stct->add_field_type(type_mapping(f));
        }
        ictx.struct_decls.push_back(stct);
    }
}

void mir2sir::emit_func_decl(const mir_context& mctx) {
    for(auto i : mctx.decls) {
        auto func = new sir_func(i->name);
        func->set_return_type(type_mapping(i->return_type));
        for(const auto& j : i->params) {
            func->add_param(j.first + ".param", type_mapping(j.second));
        }
        ictx.func_decls.push_back(func);
    }
}

void mir2sir::emit_func_impl(const mir_context& mctx) {
    for(auto i : mctx.impls) {
        auto func = new sir_func(i->name);
        func->set_return_type(type_mapping(i->return_type));
        // push local scope
        locals.push();
        locals.local_scope_counter = 0;
        // load parameters and locals
        for(const auto& j : i->params) {
            func->add_param(j.first + ".param", type_mapping(j.second));
            locals.elem.back().insert({j.first, j.first});
        }

        // generate code block
        func->set_code_block(new sir_block);
        // init ssa generator
        ssa_gen.clear();
        // clear value stack
        value_stack.clear();

        block = func->get_code_block();
        for(const auto& j : i->params) {
            block->add_alloca(new sir_alloca(j.first, type_mapping(j.second)));
            block->add_stmt(new sir_store(
                type_mapping(j.second),
                value_t::variable(j.first + ".param"),
                value_t::variable(j.first)
            ));
        }
        // visit mir and generate sir
        i->block->accept(this);

        // clear block pointer
        block = nullptr;

        ictx.func_impls.push_back(func);

        // pop local scope
        locals.pop();
    }
}

void mir2sir::generate_and(mir_binary* node) {
    auto temp_0 = ssa_gen.create();
    block->add_alloca(new sir_alloca("real." + temp_0, "i1"));
    block->add_stmt(new sir_temp_ptr(temp_0, "i1"));

    node->get_left()->accept(this);
    auto left = value_stack.back();
    value_stack.pop_back();
    block->add_stmt(new sir_store(
        "i1",
        left.to_value_t(),
        value_t::variable(temp_0)
    ));

    auto true_label = block->stmt_size() + 1;
    auto br = new sir_br_cond(
        left.content,
        true_label,
        0
    );
    block->add_stmt(br);
    block->add_stmt(new sir_label(true_label, "and.true"));

    node->get_right()->accept(this);
    auto right = value_stack.back();
    value_stack.pop_back();
    block->add_stmt(new sir_store(
        "i1",
        right.to_value_t(),
        value_t::variable(temp_0)
    ));

    auto next_label = block->stmt_size() + 1;
    block->add_stmt(new sir_br(next_label));

    auto false_label = block->stmt_size();
    br->set_false_label(false_label);

    block->add_stmt(new sir_label(next_label, "and.false"));
    auto temp_1 = ssa_gen.create();
    block->add_stmt(new sir_load(
        "i1",
        value_t::variable(temp_0),
        value_t::variable(temp_1)
    ));

    value_stack.push_back(mir_value_t::variable(temp_1, node->get_type()));
}

void mir2sir::generate_or(mir_binary* node) {
    auto temp_0 = ssa_gen.create();
    block->add_alloca(new sir_alloca("real." + temp_0, "i1"));
    block->add_stmt(new sir_temp_ptr(temp_0, "i1"));

    node->get_left()->accept(this);
    auto left = value_stack.back();
    value_stack.pop_back();
    block->add_stmt(new sir_store(
        "i1",
        left.to_value_t(),
        value_t::variable(temp_0)
    ));

    auto false_label = block->stmt_size() + 1;
    auto br = new sir_br_cond(
        left.content,
        0,
        false_label
    );
    block->add_stmt(br);
    block->add_stmt(new sir_label(false_label, "or.false"));

    node->get_right()->accept(this);
    auto right = value_stack.back();
    value_stack.pop_back();
    block->add_stmt(new sir_store(
        "i1",
        right.to_value_t(),
        value_t::variable(temp_0)
    ));

    auto next_label = block->stmt_size() + 1;
    block->add_stmt(new sir_br(next_label));

    auto true_label = block->stmt_size();
    br->set_true_label(true_label);

    block->add_stmt(new sir_label(next_label, "or.true"));
    auto temp_1 = ssa_gen.create();
    block->add_stmt(new sir_load(
        "i1",
        value_t::variable(temp_0),
        value_t::variable(temp_1)
    ));

    value_stack.push_back(mir_value_t::variable(temp_1, node->get_type()));
}

void mir2sir::visit_mir_block(mir_block* node) {
    // push local scope
    locals.push();
    for(auto i : node->get_content()) {
        i->accept(this);
    }
    // pop local scope
    locals.pop();
}

void mir2sir::visit_mir_unary(mir_unary* node) {
    node->get_value()->accept(this);
    auto source = value_stack.back();
    value_stack.pop_back();

    auto temp_var = ssa_gen.create();
    if (node->get_opr()==mir_unary::opr_kind::neg) {
        block->add_stmt(new sir_neg(
            source.to_value_t(),
            value_t::variable(temp_var),
            source.resolve_type.is_integer(),
            type_mapping(source.resolve_type)
        ));        
    } else if (node->get_opr()==mir_unary::opr_kind::bnot) {
        block->add_stmt(new sir_bnot(
            source.to_value_t(),
            value_t::variable(temp_var),
            type_mapping(source.resolve_type)
        ));
    } else if (node->get_opr()==mir_unary::opr_kind::lnot) {
        block->add_stmt(new sir_lnot(
            source.to_value_t(),
            value_t::variable(temp_var),
            type_mapping(source.resolve_type)
        ));
    }
    value_stack.push_back(mir_value_t::variable(temp_var, node->get_type()));
}

void mir2sir::visit_mir_binary(mir_binary* node) {
    if (node->get_opr()==mir_binary::opr_kind::cmpand) {
        generate_and(node);
        return;
    } else if (node->get_opr()==mir_binary::opr_kind::cmpor) {
        generate_or(node);
        return;
    }

    node->get_left()->accept(this);
    auto left = value_stack.back();
    value_stack.pop_back();

    node->get_right()->accept(this);
    auto right = value_stack.back();
    value_stack.pop_back();

    auto temp_var = ssa_gen.create();
    switch(node->get_opr()) {
        case mir_binary::opr_kind::add:
            block->add_stmt(new sir_add(
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                left.resolve_type.is_integer(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::sub:
            block->add_stmt(new sir_sub(
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                left.resolve_type.is_integer(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::mult:
            block->add_stmt(new sir_mul(
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                left.resolve_type.is_integer(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::div:
            block->add_stmt(new sir_div(
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::rem:
            block->add_stmt(new sir_rem(
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::band:
            block->add_stmt(new sir_band(
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::bxor:
            block->add_stmt(new sir_bxor(
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::bor:
            block->add_stmt(new sir_bor(
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::cmpeq: {
            auto flag_is_integer = left.resolve_type.is_integer() ||
                left.resolve_type.is_pointer() ||
                left.resolve_type.is_boolean() ||
                ctx.search_symbol_kind(left.resolve_type)==sym_kind::enum_kind;
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_eq,
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                flag_is_integer,
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
        } break;
        case mir_binary::opr_kind::cmpneq: {
            auto flag_is_integer = left.resolve_type.is_integer() ||
                left.resolve_type.is_pointer() ||
                left.resolve_type.is_boolean() ||
                ctx.search_symbol_kind(left.resolve_type)==sym_kind::enum_kind;
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_neq,
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                flag_is_integer,
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
        } break;
        case mir_binary::opr_kind::grt:
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_gt,
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::geq:
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_ge,
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::less:
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_lt,
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::leq:
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_le,
                left.to_value_t(),
                right.to_value_t(),
                value_t::variable(temp_var),
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        default: unimplemented(node); break;
    }

    value_stack.push_back(mir_value_t::variable(temp_var, node->get_type()));
}

void mir2sir::visit_mir_type_convert(mir_type_convert* node) {
    node->get_source()->accept(this);
    auto source = value_stack.back();
    value_stack.pop_back();

    auto temp_var = ssa_gen.create();
    block->add_stmt(new sir_type_convert(
        source.to_value_t(),
        value_t::variable(temp_var),
        type_mapping(source.resolve_type),
        type_mapping(node->get_target_type())
    ));
    value_stack.push_back(mir_value_t::variable(
        temp_var,
        node->get_target_type()
    ));
}

void mir2sir::visit_mir_nil(mir_nil* node) {
    value_stack.push_back(mir_value_t::nil(node->get_type()));
}

void mir2sir::visit_mir_number(mir_number* node) {
    auto literal = node->get_literal();
    if (!node->get_type().is_integer() &&
        literal.find('.')==std::string::npos) {
        literal += ".0";
    }
    value_stack.push_back(mir_value_t::literal(literal, node->get_type()));
}

void mir2sir::visit_mir_string(mir_string* node) {
    auto temp_var = ssa_gen.create();
    block->add_stmt(new sir_string(
        node->get_literal().length() + 1,
        ictx.const_strings.at(node->get_literal()),
        value_t::variable(temp_var)
    ));
    value_stack.push_back(mir_value_t::variable(temp_var, node->get_type()));
}

void mir2sir::visit_mir_char(mir_char* node) {
    const auto literal = std::to_string(u32(node->get_literal()));
    value_stack.push_back(mir_value_t::literal(literal, node->get_type()));
}

void mir2sir::visit_mir_bool(mir_bool* node) {
    value_stack.push_back(mir_value_t::literal(
        node->get_literal()? "1":"0",
        node->get_type()
    ));
}

void mir2sir::call_expression_generation(mir_call* node, bool need_address) {
    node->get_content()->accept(this);

    auto source = value_stack.back();
    // for enum member
    if (!source.resolve_type.is_pointer()) {
        return;
    }
    if (source.resolve_type==type::void_type()) {
        value_stack.pop_back();
        return;
    }

    if (need_address) {
        return;
    }

    value_stack.pop_back();

    auto temp_var = ssa_gen.create();
    block->add_stmt(new sir_load(
        type_mapping(source.resolve_type.get_ref_copy()),
        source.to_value_t(),
        value_t::variable(temp_var)
    ));

    value_stack.push_back(mir_value_t::variable(
        temp_var,
        source.resolve_type.get_ref_copy()
    ));
}

void mir2sir::visit_mir_call(mir_call* node) {
    call_expression_generation(node, false);
}

void mir2sir::visit_mir_call_id(mir_call_id* node) {
    if (!node->get_type().is_global) {
        value_stack.push_back(mir_value_t::variable(
            locals.get_local(node->get_name()),
            node->get_type().get_pointer_copy()
        ));
        return;
    }

    // get full path
    switch(ctx.search_symbol_kind(node->get_type())) {
        case sym_kind::func_kind:
            value_stack.push_back(mir_value_t::func_kind(
                node->get_type().name,
                node->get_type()
            ));
            break;
        case sym_kind::struct_kind:
            value_stack.push_back(mir_value_t::struct_kind(
                node->get_type().full_path_name(),
                node->get_type()
            ));
            break;
        case sym_kind::enum_kind:
            value_stack.push_back(mir_value_t::enum_kind(
                node->get_type().full_path_name(),
                node->get_type()
            ));
            break;
        default: unimplemented(node); break;
    }
}

void mir2sir::visit_mir_call_index(mir_call_index* node) {
    auto prev = value_stack.back();
    value_stack.pop_back();

    node->get_index()->accept(this);
    auto index = value_stack.back();
    value_stack.pop_back();

    auto temp_var = ssa_gen.create();
    block->add_stmt(new sir_load(
        type_mapping(prev.resolve_type.get_ref_copy()),
        prev.to_value_t(),
        value_t::variable(temp_var)
    ));

    auto target = ssa_gen.create();
    block->add_stmt(new sir_call_index(
        value_t::variable(temp_var),
        value_t::variable(target),
        index.to_value_t(),
        type_mapping(prev.resolve_type.get_ref_copy().get_ref_copy()),
        type_mapping(index.resolve_type)
    ));
    
    value_stack.push_back(mir_value_t::variable(
        target,
        node->get_type().get_pointer_copy()
    ));
}

void mir2sir::visit_mir_call_func(mir_call_func* node) {
    auto prev = value_stack.back();
    value_stack.pop_back();

    std::vector<mir_value_t> args;
    if (prev.value_kind==mir_value_t::kind::method) {
        args.push_back(value_stack.back());
        value_stack.pop_back();
    }

    for(auto i : node->get_args()->get_content()) {
        i->accept(this);
        args.push_back(value_stack.back());
        value_stack.pop_back();        
    }

    std::string target = "";
    sir_call_func* sir_function_call = nullptr;
    if (node->get_type()!=type::void_type()) {
        target = ssa_gen.create();
        sir_function_call = new sir_call_func(
            mangle(prev.content),
            type_mapping(node->get_type()),
            value_t::variable(target)
        );
    } else {
        sir_function_call = new sir_call_func(
            mangle(prev.content),
            type_mapping(node->get_type()),
            value_t::null()
        );
    }

    // load args
    for(const auto& i : args) {
        sir_function_call->add_arg(i.to_value_t());
        sir_function_call->add_arg_type(type_mapping(i.resolve_type));
    }
    block->add_stmt(sir_function_call);

    if (node->get_type()!=type::void_type()) {
        auto temp_var = ssa_gen.create();
        block->add_alloca(new sir_alloca(
            "real." + temp_var,
            type_mapping(node->get_type())
        ));
        block->add_stmt(new sir_temp_ptr(
            temp_var,
            type_mapping(node->get_type())
        ));
        block->add_stmt(new sir_store(
            type_mapping(node->get_type()),
            value_t::variable(target),
            value_t::variable(temp_var)
        ));
        value_stack.push_back(mir_value_t::variable(
            temp_var,
            node->get_type().get_pointer_copy()
        ));
    } else {
        value_stack.push_back(mir_value_t::variable(
            target,
            node->get_type()
        ));
    }
}

void mir2sir::visit_mir_get_field(mir_get_field* node) {
    auto prev = value_stack.back();
    value_stack.pop_back();

    const auto& dm = ctx.global.domain.at(prev.resolve_type.loc_file);
    const auto& st = dm.structs.at(prev.resolve_type.name);

    // get method
    if (st.method.count(node->get_name())) {
        // push self into the stack
        value_stack.push_back(prev);
        value_stack.push_back(mir_value_t::method(
            prev.resolve_type.full_path_name() + "::" + node->get_name(),
            node->get_type()
        ));
        return;
    }

    const auto target = ssa_gen.create();
    const auto index = st.field_index(node->get_name());
    block->add_stmt(new sir_call_field(
        target,
        prev.content,
        type_mapping(prev.resolve_type.get_ref_copy()),
        index
    ));

    value_stack.push_back(mir_value_t::variable(
        target,
        node->get_type().get_pointer_copy()
    ));
}

void mir2sir::visit_mir_get_path(mir_get_path* node) {
    auto prev = value_stack.back();
    value_stack.pop_back();

    switch(prev.value_kind) {
        case mir_value_t::kind::struct_symbol:
            value_stack.push_back(mir_value_t::func_kind(
                prev.resolve_type.full_path_name() + "::" + node->get_name(),
                node->get_type()
            ));
            break;
        case mir_value_t::kind::enum_symbol: {
            const auto& dm = ctx.global.domain.at(prev.resolve_type.loc_file);
            const auto& em = dm.enums.at(prev.resolve_type.name);
            value_stack.push_back(mir_value_t::literal(
                std::to_string(em.members.at(node->get_name())),
                node->get_type()
            ));
        } break;
        default: unimplemented(node); break;
    }
}

void mir2sir::visit_mir_ptr_get_field(mir_ptr_get_field* node) {
    auto prev = value_stack.back();
    value_stack.pop_back();

    const auto& dm = ctx.global.domain.at(prev.resolve_type.loc_file);
    const auto& st = dm.structs.at(prev.resolve_type.name);

    // get method
    if (st.method.count(node->get_name())) {
        // push self into the stack
        auto temp_var = ssa_gen.create();
        block->add_stmt(new sir_load(
            type_mapping(prev.resolve_type.get_ref_copy()),
            prev.to_value_t(),
            value_t::variable(temp_var)
        ));
        value_stack.push_back(mir_value_t::variable(
            temp_var,
            prev.resolve_type.get_ref_copy()
        ));
        value_stack.push_back(mir_value_t::method(
            prev.resolve_type.full_path_name() + "::" + node->get_name(),
            node->get_type()
        ));
        return;
    }

    const auto index = st.field_index(node->get_name());
    const auto temp_0 = ssa_gen.create();
    const auto temp_1 = ssa_gen.create();
    block->add_stmt(new sir_load(
        type_mapping(prev.resolve_type.get_ref_copy()),
        prev.to_value_t(),
        value_t::variable(temp_0)
    ));
    block->add_stmt(new sir_call_field(
        temp_1,
        temp_0,
        type_mapping(prev.resolve_type.get_ref_copy().get_ref_copy()),
        index
    ));
    value_stack.push_back(mir_value_t::variable(
        temp_1,
        node->get_type().get_pointer_copy()
    ));
}

void mir2sir::visit_mir_define(mir_define* node) {
    const auto name = node->get_name() +
                      "." + std::to_string(locals.local_scope_counter);
    locals.elem.back().insert({
        node->get_name(),
        name
    });

    block->add_alloca(new sir_alloca(
        name,
        type_mapping(node->get_type())
    ));

    node->get_init_value()->accept(this);
    auto source = value_stack.back();
    value_stack.pop_back();

    block->add_stmt(new sir_store(
        type_mapping(node->get_type()),
        source.to_value_t(),
        value_t::variable(name)
    ));
}

void mir2sir::visit_mir_assign(mir_assign* node) {
    call_expression_generation(
        reinterpret_cast<mir_call*>(node->get_left()->get_content().front()),
        true
    );
    auto left = value_stack.back();
    value_stack.pop_back();

    node->get_right()->accept(this);
    auto right = value_stack.back();
    value_stack.pop_back();

    switch(node->get_opr()) {
        case mir_assign::opr_kind::addeq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(right.resolve_type),
                left.to_value_t(),
                value_t::variable(temp_0)
            ));
            block->add_stmt(new sir_add(
                value_t::variable(temp_0),
                right.to_value_t(),
                value_t::variable(temp_1),
                left.resolve_type.is_integer(),
                type_mapping(right.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(right.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::subeq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(right.resolve_type),
                left.to_value_t(),
                value_t::variable(temp_0)
            ));
            block->add_stmt(new sir_sub(
                value_t::variable(temp_0),
                right.to_value_t(),
                value_t::variable(temp_1),
                left.resolve_type.is_integer(),
                type_mapping(right.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(right.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::multeq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(right.resolve_type),
                left.to_value_t(),
                value_t::variable(temp_0)
            ));
            block->add_stmt(new sir_mul(
                value_t::variable(temp_0),
                right.to_value_t(),
                value_t::variable(temp_1),
                left.resolve_type.is_integer(),
                type_mapping(right.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(right.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::diveq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(right.resolve_type),
                left.to_value_t(),
                value_t::variable(temp_0)
            ));
            block->add_stmt(new sir_div(
                value_t::variable(temp_0),
                right.to_value_t(),
                value_t::variable(temp_1),
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(right.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(right.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::remeq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(right.resolve_type),
                left.to_value_t(),
                value_t::variable(temp_0)
            ));
            block->add_stmt(new sir_rem(
                value_t::variable(temp_0),
                right.to_value_t(),
                value_t::variable(temp_1),
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(right.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(right.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::eq:
            block->add_stmt(new sir_store(
                type_mapping(right.resolve_type),
                right.to_value_t(),
                left.to_value_t()
            ));
            break;
        case mir_assign::opr_kind::andeq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(right.resolve_type),
                left.to_value_t(),
                value_t::variable(temp_0)
            ));
            block->add_stmt(new sir_band(
                value_t::variable(temp_0),
                right.to_value_t(),
                value_t::variable(temp_1),
                type_mapping(right.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(right.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::xoreq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(right.resolve_type),
                left.to_value_t(),
                value_t::variable(temp_0)
            ));
            block->add_stmt(new sir_bxor(
                value_t::variable(temp_0),
                right.to_value_t(),
                value_t::variable(temp_1),
                type_mapping(right.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(right.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::oreq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(right.resolve_type),
                left.to_value_t(),
                value_t::variable(temp_0)
            ));
            block->add_stmt(new sir_bor(
                value_t::variable(temp_0),
                right.to_value_t(),
                value_t::variable(temp_1),
                type_mapping(right.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(right.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
    }
}

void mir2sir::visit_mir_if(mir_if* node) {
    sir_br_cond* br_cond = nullptr;
    if (node->get_condition()) {
        node->get_condition()->accept(this);
        auto cond = value_stack.back();
        value_stack.pop_back();
        br_cond = new sir_br_cond(
            cond.content,
            block->stmt_size() + 1,
            0
        );
        block->add_stmt(br_cond);
        block->add_stmt(new sir_label(block->stmt_size(), "cond.true"));
    }
    node->get_content()->accept(this);

    // for block ends with ret instruction, another basic block is needed
    // because ret instruction is the terminator instruction
    if (block->back_is_ret_stmt()) {
        block->add_stmt(new sir_place_holder_label(block->stmt_size()));
    }

    auto jump_out = new sir_br(0);
    branch_jump_out.back().push_back(jump_out);
    block->add_stmt(jump_out);

    if (br_cond) {
        br_cond->set_false_label(block->stmt_size());
        block->add_stmt(new sir_label(block->stmt_size(), "cond.false"));
    }
}

void mir2sir::visit_mir_branch(mir_branch* node) {
    branch_jump_out.push_back({});

    for(auto i : node->get_branch()) {
        i->accept(this);
        if (i->get_condition() && i==node->get_branch().back()) {
            block->add_stmt(new sir_br(block->stmt_size()+1));
        }
    }

    for(auto i : branch_jump_out.back()) {
        i->set_label(block->stmt_size());
    }
    branch_jump_out.pop_back();

    block->add_stmt(new sir_label(block->stmt_size(), "branch.end"));
}

void mir2sir::visit_mir_break(mir_break* node) {
    auto break_br = new sir_br(0);
    break_inst.back().push_back(break_br);
    block->add_stmt(break_br);
    block->add_stmt(new sir_place_holder_label(block->stmt_size()));
}

void mir2sir::visit_mir_continue(mir_continue* node) {
    block->add_stmt(new sir_br(loop_entry.back()));
    block->add_stmt(new sir_place_holder_label(block->stmt_size()));
}

void mir2sir::visit_mir_loop(mir_loop* node) {
    auto entry_label = block->stmt_size() + 1;
    loop_entry.push_back(entry_label);
    break_inst.push_back({});

    block->add_stmt(new sir_br(entry_label));
    block->add_stmt(new sir_label(entry_label, "loop.entry"));

    node->get_condition()->accept(this);
    auto cond = value_stack.back();
    value_stack.pop_back();

    auto cond_ir = new sir_br_cond(
        cond.content,
        block->stmt_size() + 1,
        0
    );
    block->add_stmt(cond_ir);
    block->add_stmt(new sir_label(block->stmt_size(), "loop.cond.true"));

    node->get_content()->accept(this);
    block->add_stmt(new sir_br(entry_label));

    auto exit_label = block->stmt_size();
    cond_ir->set_false_label(exit_label);
    block->add_stmt(new sir_label(exit_label, "loop.exit"));
    for(auto i : break_inst.back()) {
        i->set_label(exit_label);
    }

    loop_entry.pop_back();
    break_inst.pop_back();
}

void mir2sir::visit_mir_return(mir_return* node) {
    if (node->get_value()->get_content().empty()) {
        block->add_stmt(new sir_ret(
            "void",
            value_t::null()
        ));
        return;
    }

    node->get_value()->accept(this);
    auto ret = value_stack.back();
    value_stack.pop_back();

    block->add_stmt(new sir_ret(
        type_mapping(ret.resolve_type),
        ret.to_value_t()
    ));
}

const error& mir2sir::generate(const mir_context& mctx) {
    generate_type_mapper();
    for(const auto& i : mctx.const_strings) {
        ictx.const_strings.insert(i);
    }
    emit_struct(mctx);
    emit_func_decl(mctx);
    emit_func_impl(mctx);

    return err;
}

}
