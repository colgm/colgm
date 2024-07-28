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
            type_mapper.insert({tp.full_path_name(), symbol_kind::enum_kind});
        }
        for(const auto& i : dm.second.structs) {
            const auto tp = type {
                .name = i.second.name,
                .loc_file = i.second.location.file
            };
            type_mapper.insert({tp.full_path_name(), symbol_kind::struct_kind});
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
        case symbol_kind::struct_kind:
            copy.name = "%struct." + mangle(full_name);
            break;
        case symbol_kind::enum_kind: copy = type::i64_type(); break;
        default: break;
    }
    return copy.to_string();
}

void mir2sir::emit_struct(const mir_context& mctx) {
    for(auto i : mctx.structs) {
        auto stct = hir_struct(i->name, i->location);
        for(const auto& f : i->field_type) {
            stct.add_field_type(type_mapping(f));
        }
        ictx.struct_decls.push_back(stct);
    }
}

void mir2sir::emit_func_decl(const mir_context& mctx) {
    for(auto i : mctx.decls) {
        auto func = new hir_func(i->name);
        func->set_return_type(type_mapping(i->return_type));
        for(const auto& j : i->params) {
            func->add_param(j.first + ".param", type_mapping(j.second));
        }
        ictx.func_decls.push_back(func);
    }
}

void mir2sir::emit_func_impl(const mir_context& mctx) {
    for(auto i : mctx.impls) {
        auto func = new hir_func(i->name);
        func->set_return_type(type_mapping(i->return_type));
        for(const auto& j : i->params) {
            func->add_param(j.first + ".param", type_mapping(j.second));
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
    }
}

void mir2sir::visit_mir_nop(mir_nop* node) {
    block->add_nop();
}

void mir2sir::visit_mir_block(mir_block* node) {
    for(auto i : node->get_content()) {
        i->accept(this);
    }
}

void mir2sir::visit_mir_unary(mir_unary* node) {
    node->get_value()->accept(this);
    auto source = value_stack.back();
    value_stack.pop_back();

    auto temp_var = ssa_gen.create();
    if (node->get_opr()==mir_unary::opr_kind::neg) {
        block->add_stmt(new sir_neg(
            source.content,
            temp_var,
            source.resolve_type.is_integer(),
            type_mapping(source.resolve_type)
        ));        
    } else if (node->get_opr()==mir_unary::opr_kind::bnot) {
        block->add_stmt(new sir_bnot(
            source.content,
            temp_var,
            type_mapping(source.resolve_type)
        ));
    }
    value_stack.push_back(mir_value_t::variable(temp_var, node->get_type()));
}

void mir2sir::visit_mir_binary(mir_binary* node) {
    if (node->get_opr()==mir_binary::opr_kind::cmpand) {
        // TODO
        return;
    } else if (node->get_opr()==mir_binary::opr_kind::cmpor) {
        // TODO
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
                left.content,
                right.content,
                temp_var,
                left.resolve_type.is_integer(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::sub:
            block->add_stmt(new sir_sub(
                left.content,
                right.content,
                temp_var,
                left.resolve_type.is_integer(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::mult:
            block->add_stmt(new sir_mul(
                left.content,
                right.content,
                temp_var,
                left.resolve_type.is_integer(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::div:
            block->add_stmt(new sir_div(
                left.content,
                right.content,
                temp_var,
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::rem:
            block->add_stmt(new sir_rem(
                left.content,
                right.content,
                temp_var,
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::band:
            block->add_stmt(new sir_band(
                left.content,
                right.content,
                temp_var,
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::bxor:
            block->add_stmt(new sir_bxor(
                left.content,
                right.content,
                temp_var,
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::bor:
            block->add_stmt(new sir_bor(
                left.content,
                right.content,
                temp_var,
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::cmpeq: {
            auto flag_is_integer = left.resolve_type.is_integer() ||
            ctx.search_symbol_kind(left.resolve_type)==symbol_kind::enum_kind;
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_eq,
                left.content,
                right.content,
                temp_var,
                flag_is_integer,
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
        } break;
        case mir_binary::opr_kind::cmpneq: {
            auto flag_is_integer = left.resolve_type.is_integer() ||
            ctx.search_symbol_kind(left.resolve_type)==symbol_kind::enum_kind;
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_neq,
                left.content,
                right.content,
                temp_var,
                flag_is_integer,
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
        } break;
        case mir_binary::opr_kind::grt:
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_gt,
                left.content,
                right.content,
                temp_var,
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::geq:
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_ge,
                left.content,
                right.content,
                temp_var,
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::less:
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_lt,
                left.content,
                right.content,
                temp_var,
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            break;
        case mir_binary::opr_kind::leq:
            block->add_stmt(new sir_cmp(
                sir_cmp::kind::cmp_le,
                left.content,
                right.content,
                temp_var,
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
        source.content,
        temp_var,
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
        node->get_literal(),
        ictx.const_strings.at(node->get_literal()),
        temp_var
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

void mir2sir::visit_mir_call(mir_call* node) {
    node->get_content()->accept(this);
}

void mir2sir::visit_mir_call_id(mir_call_id* node) {
    if (!node->get_type().is_global) {
        value_stack.push_back(mir_value_t::variable(
            node->get_name(),
            node->get_type().get_pointer_copy()
        ));
        return;
    }

    // get full path
    switch(ctx.search_symbol_kind(node->get_type())) {
        case symbol_kind::func_kind:
            value_stack.push_back(mir_value_t::func_kind(
                node->get_type().name,
                node->get_type()
            ));
            break;
        case symbol_kind::struct_kind:
            value_stack.push_back(mir_value_t::struct_kind(
                node->get_type().full_path_name(),
                node->get_type()
            ));
            break;
        case symbol_kind::enum_kind:
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

    auto target = ssa_gen.create();
    block->add_stmt(new sir_call_index(
        prev.content,
        target,
        index.content,
        type_mapping(prev.resolve_type),
        type_mapping(index.resolve_type)
    ));
    
    value_stack.push_back(mir_value_t::variable(target, node->get_type()));
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

    auto target = ssa_gen.create();
    auto sir_function_call = new sir_call_func(
        mangle(prev.content),
        type_mapping(node->get_type()),
        target
    );
    for(const auto& i : args) {
        sir_function_call->add_arg(i.content);
        sir_function_call->add_arg_type(type_mapping(i.resolve_type));
    }
    block->add_stmt(sir_function_call);
    value_stack.push_back(mir_value_t::variable(target, node->get_type()));
}

void mir2sir::visit_mir_get_field(mir_get_field* node) {
    auto prev = value_stack.back();
    value_stack.pop_back();

    const auto& dm = ctx.global.domain.at(prev.resolve_type.loc_file);
    const auto& st = dm.structs.at(prev.resolve_type.name);

    if (st.method.count(node->get_name())) {
        value_stack.push_back(prev);
        value_stack.push_back(mir_value_t::method(
            prev.resolve_type.full_path_name() + "::" + node->get_name(),
            node->get_type()
        ));
        return;
    }

    const auto target = ssa_gen.create();
    usize index = 0;
    for(; index < st.ordered_field.size(); ++index) {
        if (st.ordered_field[index].name==node->get_name()) {
            break;
        }
    }

    block->add_stmt(new sir_call_field(
        target,
        prev.content,
        type_mapping(prev.resolve_type),
        index
    ));

    value_stack.push_back(mir_value_t::variable(target, node->get_type()));
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

    if (st.method.count(node->get_name())) {
        value_stack.push_back(prev);
        value_stack.push_back(mir_value_t::method(
            prev.resolve_type.full_path_name() + "::" + node->get_name(),
            node->get_type()
        ));
        return;
    }

    const auto target = ssa_gen.create();
    usize index = 0;
    for(; index < st.ordered_field.size(); ++index) {
        if (st.ordered_field[index].name==node->get_name()) {
            break;
        }
    }

    block->add_stmt(new sir_call_field(
        target,
        prev.content,
        type_mapping(prev.resolve_type),
        index
    ));

    value_stack.push_back(mir_value_t::variable(target, node->get_type()));
}


void mir2sir::visit_mir_define(mir_define* node) {
    block->add_alloca(new sir_alloca(
        node->get_name(),
        type_mapping(node->get_resolve_type())
    ));

    node->get_init_value()->accept(this);
    auto source = value_stack.back();
    value_stack.pop_back();

    block->add_stmt(new sir_store(
        type_mapping(node->get_resolve_type()),
        source.to_value_t(),
        value_t::variable(node->get_name())
    ));
}

void mir2sir::visit_mir_assign(mir_assign* node) {
    in_assign_lvalue_process = true;
    node->get_left()->accept(this);
    in_assign_lvalue_process = false;
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
                type_mapping(left.resolve_type),
                left.content,
                temp_0
            ));
            block->add_stmt(new sir_add(
                temp_0,
                right.content,
                temp_1,
                left.resolve_type.is_integer(),
                type_mapping(left.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(left.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::subeq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(left.resolve_type),
                left.content,
                temp_0
            ));
            block->add_stmt(new sir_sub(
                temp_0,
                right.content,
                temp_1,
                left.resolve_type.is_integer(),
                type_mapping(left.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(left.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::multeq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(left.resolve_type),
                left.content,
                temp_0
            ));
            block->add_stmt(new sir_mul(
                temp_0,
                right.content,
                temp_1,
                left.resolve_type.is_integer(),
                type_mapping(left.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(left.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::diveq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(left.resolve_type),
                left.content,
                temp_0
            ));
            block->add_stmt(new sir_div(
               temp_0,
               right.content,
               temp_1,
               left.resolve_type.is_integer(),
               !left.resolve_type.is_unsigned(),
               type_mapping(left.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(left.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::remeq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(left.resolve_type),
                left.content,
                temp_0
            ));
            block->add_stmt(new sir_rem(
                temp_0,
                right.content,
                temp_1,
                left.resolve_type.is_integer(),
                !left.resolve_type.is_unsigned(),
                type_mapping(left.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(left.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::eq:
            block->add_stmt(new sir_store(
                type_mapping(left.resolve_type),
                right.to_value_t(),
                left.to_value_t()
            ));
            break;
        case mir_assign::opr_kind::andeq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(left.resolve_type),
                left.content,
                temp_0
            ));
            block->add_stmt(new sir_band(
                temp_0,
                right.content,
                temp_1,
                type_mapping(left.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(left.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::xoreq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(left.resolve_type),
                left.content,
                temp_0
            ));
            block->add_stmt(new sir_bxor(
               temp_0,
               right.content,
               temp_1,
               type_mapping(left.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(left.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
        case mir_assign::opr_kind::oreq: {
            auto temp_0 = ssa_gen.create();
            auto temp_1 = ssa_gen.create();
            block->add_stmt(new sir_load(
                type_mapping(left.resolve_type),
                left.content,
                temp_0
            ));
            block->add_stmt(new sir_bor(
                temp_0,
                right.content,
                temp_1,
                type_mapping(left.resolve_type)
            ));
            block->add_stmt(new sir_store(
                type_mapping(left.resolve_type),
                value_t::variable(temp_1),
                left.to_value_t()
            ));
        } break;
    }
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

    std::ofstream out("test_mir2sir.dump.ll");
    ictx.dump_code(out);
    return err;
}

}
