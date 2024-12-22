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
    // basic type mapping
    if (basic_type_mapper.count(copy.name)) {
        copy.name = basic_type_mapper.at(copy.name);
        return copy.to_string();
    }

    const auto full_name = t.full_path_name();
    // if not found, let it crash
    if (!type_mapper.count(full_name)) {
        return "undef." + full_name;
    }
    switch(type_mapper.at(full_name)) {
        case sym_kind::struct_kind:
            // here we use full path name without generic info
            // otherwise for example:
            // std::vec<data::foo>
            //
            // will be wrongly mapped to
            // %struct.std.vec<data.foo><data::foo>
            //
            // but expect to be
            // %struct.std.vec<data::foo>
            copy.name = "%struct." + mangle(t.full_path_name(false));
            break;
        case sym_kind::enum_kind:
            // should copy pointer depth too
            copy = type::i64_type(copy.pointer_depth);
            break;
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
        func->set_attributes(i->attributes);
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
        func->set_attributes(i->attributes);
        func->set_return_type(type_mapping(i->return_type));
        // push local scope
        locals.push();
        locals.local_scope_counter = 0;
        // load parameters and locals
        for(const auto& j : i->params) {
            func->add_param(j.first + ".param", type_mapping(j.second));
            locals.elem.back().insert({j.first, j.first});
        }
        // if having debug info, set it
        if (dwarf_status.impl_debug_info.count(i->name)) {
            auto scope = dwarf_status.impl_debug_info.at(i->name);
            func->set_debug_info_index(scope);
            dwarf_status.scope_index = scope;
        } else {
            dwarf_status.scope_index = DI_node::DI_ERROR_INDEX;
        }

        // generate code block
        func->set_code_block(new sir_block);
        // init ssa generator
        ssa_gen.clear();
        array_ssa_gen.clear();
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
    block->add_move_register(new sir_alloca("_" + temp_0 + ".real", "i1"));
    block->add_stmt(new sir_temp_ptr(temp_0, "_" + temp_0 + ".real", "i1"));

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
    block->add_move_register(new sir_alloca("_" + temp_0 + ".real", "i1"));
    block->add_stmt(new sir_temp_ptr(temp_0, "_" + temp_0 + ".real", "i1"));

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

void mir2sir::visit_mir_array(mir_array* node) {
    auto index = array_ssa_gen.create();
    auto temp_0 = "arr." + index + ".ptr";
    auto temp_1 = "arr." + index + ".cast_ptr";
    const auto array_type = "[" + std::to_string(node->get_size()) + " x " +
                            type_mapping(node->get_type().get_ref_copy()) + "]";
    auto array_new = new sir_alloca(
        temp_0,
        array_type
    );
    block->add_alloca(array_new);
    block->add_stmt(new sir_type_convert(
        value_t::variable(temp_0),
        value_t::variable(temp_1),
        array_type + "*",
        type_mapping(node->get_type())
    ));
    value_stack.push_back(mir_value_t::variable(temp_1, node->get_type()));
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

void mir2sir::visit_mir_struct_init(mir_struct_init* node) {
    const auto temp_var = ssa_gen.create();
    block->add_move_register(new sir_alloca(
        "_" + temp_var + ".real",
        type_mapping(node->get_type())
    ));
    block->add_stmt(new sir_temp_ptr(
        temp_var,
        "_" + temp_var + ".real",
        type_mapping(node->get_type())
    ));
    block->add_stmt(new sir_zeroinitializer(
        value_t::variable(temp_var),
        type_mapping(node->get_type())
    ));

    const auto& dm = ctx.global.domain.at(node->get_type().loc_file);
    const auto& st = dm.structs.at(node->get_type().name_for_search());
    for(const auto& i : node->get_fields()) {
        const auto target = ssa_gen.create();
        const auto index = st.field_index(i.name);
        block->add_stmt(new sir_get_field(
            target,
            temp_var,
            type_mapping(node->get_type()),
            index
        ));
        i.content->accept(this);

        const auto res = value_stack.back();
        block->add_stmt(new sir_store(
            type_mapping(res.resolve_type),
            res.to_value_t(),
            value_t::variable(target)
        ));
        value_stack.pop_back();
    }
    value_stack.push_back(mir_value_t::variable(
        temp_var,
        node->get_type().get_pointer_copy()
    ));
}

void mir2sir::push_global_func(const type& t) {
    // avoid out of range error
    if (!ctx.global.domain.count(t.loc_file)) {
        value_stack.push_back(mir_value_t::func_kind(t.name, t));
        return;
    }
    const auto& dm = ctx.global.domain.at(t.loc_file);

    // avoid out of range error
    if (!dm.functions.count(t.name_for_search())) {
        value_stack.push_back(mir_value_t::func_kind(t.name, t));
        return;
    }

    const auto& fn = dm.functions.at(t.name_for_search());

    // extern function will reserve raw name, others are mangled
    if (fn.is_extern) {
        value_stack.push_back(mir_value_t::func_kind(t.name, t));
        return;
    }

    // do mangling
    const auto tmp = type {
        .name = t.name,
        .loc_file = fn.location.file,
        .generics = t.generics
    };
    value_stack.push_back(mir_value_t::func_kind(
        mangle(tmp.full_path_name()),
        t
    ));
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
        case sym_kind::basic_kind:
            value_stack.push_back(mir_value_t::primitive(
                node->get_type().full_path_name(),
                node->get_type()
            ));
            break;
        case sym_kind::func_kind:
            push_global_func(node->get_type());
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
    block->add_stmt(new sir_get_index(
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
    sir_call* sir_function_call = nullptr;
    if (node->get_type() != type::void_type()) {
        target = ssa_gen.create();
        sir_function_call = new sir_call(
            mangle(prev.content),
            type_mapping(node->get_type()),
            value_t::variable(target)
        );
    } else {
        sir_function_call = new sir_call(
            mangle(prev.content),
            type_mapping(node->get_type()),
            value_t::null()
        );
    }

    // generate DWARF info
    {
        auto DI_loc = new DI_location(
            dwarf_status.DI_counter,
            node->get_location().begin_line,
            node->get_location().begin_column,
            dwarf_status.scope_index
        );
        sir_function_call->set_debug_info_index(dwarf_status.DI_counter);
        ictx.debug_info.push_back(DI_loc);
        ++dwarf_status.DI_counter;
    }

    // load args
    for(const auto& i : args) {
        sir_function_call->add_arg(i.to_value_t());
        sir_function_call->add_arg_type(type_mapping(i.resolve_type));
    }
    block->add_stmt(sir_function_call);

    if (node->get_type()!=type::void_type()) {
        auto temp_var = ssa_gen.create();
        block->add_move_register(new sir_alloca(
            "_" + temp_var + ".real",
            type_mapping(node->get_type())
        ));
        block->add_stmt(new sir_temp_ptr(
            temp_var,
            "_" + temp_var + ".real",
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
    const auto& st = dm.structs.at(prev.resolve_type.name_for_search());

    // get method
    if (st.method.count(node->get_name())) {
        // push self into the stack
        value_stack.push_back(prev);
        value_stack.push_back(mir_value_t::method(
            prev.resolve_type.full_path_name() + "." + node->get_name(),
            node->get_type()
        ));
        return;
    }

    const auto target = ssa_gen.create();
    const auto index = st.field_index(node->get_name());
    block->add_stmt(new sir_get_field(
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
        case mir_value_t::kind::primitive:
        case mir_value_t::kind::struct_symbol:
            value_stack.push_back(mir_value_t::func_kind(
                prev.resolve_type.full_path_name() + "." + node->get_name(),
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
    const auto& st = dm.structs.at(prev.resolve_type.name_for_search());

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
            prev.resolve_type.full_path_name() + "." + node->get_name(),
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
    block->add_stmt(new sir_get_field(
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
        block->add_stmt(new sir_label(block->stmt_size(), "block.end.ret"));
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

void mir2sir::visit_mir_switch_case(mir_switch_case* node) {
    node->get_content()->accept(this);
}

void mir2sir::visit_mir_switch(mir_switch* node) {
    node->get_condition()->accept(this);

    std::vector<sir_br*> jmp_exits;

    const auto value = value_stack.back();
    value_stack.pop_back();
    auto switch_inst = new sir_switch(value.to_value_t());
    block->add_stmt(switch_inst);
    for (auto i : node->get_cases()) {
        auto case_label = block->stmt_size();
        auto label = new sir_label(
            case_label,
            "switch.case " + std::to_string(i->get_value())
        );
        block->add_stmt(label);
        switch_inst->add_case(i->get_value(), case_label);
        i->accept(this);

        // if block ends with ret instruction, do not generate
        // switch jump exit instruction
        if (block->back_is_ret_stmt()) {
            continue;
        }
        auto jmp_exit = new sir_br(0);
        block->add_stmt(jmp_exit);
        jmp_exits.push_back(jmp_exit);
    }

    auto default_label = block->stmt_size();
    auto label = new sir_label(
        default_label,
        "switch.default"
    );
    block->add_stmt(label);
    switch_inst->set_default_label(default_label);
    if (node->get_default_case()) {
        node->get_default_case()->accept(this);

        // if block ends with ret instruction, do not generate
        // switch jump exit instruction
        if (!block->back_is_ret_stmt()) {
            auto jmp_exit = new sir_br(0);
            block->add_stmt(jmp_exit);
            jmp_exits.push_back(jmp_exit);
        }
        auto exit_label = block->stmt_size();
        block->add_stmt(new sir_label(exit_label, "switch.end"));
        for (auto i : jmp_exits) {
            i->set_label(exit_label);
        }
    } else {
        for (auto i : jmp_exits) {
            i->set_label(default_label);
        }
    }
}

void mir2sir::visit_mir_break(mir_break* node) {
    auto break_br = new sir_br(0);
    break_inst.back().push_back(break_br);
    block->add_stmt(break_br);
    block->add_stmt(new sir_label(block->stmt_size(), "break.end"));
}

void mir2sir::visit_mir_continue(mir_continue* node) {
    auto continue_br = new sir_br(0);
    continue_inst.back().push_back(continue_br);
    block->add_stmt(continue_br);
    block->add_stmt(new sir_label(block->stmt_size(), "continue.end"));
}

void mir2sir::visit_mir_loop(mir_loop* node) {
    // mir loop will generate llvm ir in this form:
    //
    // %loop.entry:
    //   ... ; condition
    //   br i1 %cond %loop.cond.true, %loop.exit
    // %loop.cond.true:
    //   ... ; content
    // br %loop.continue
    // %loop.continue:   ; continue jumps here
    //   ... ; update
    // br %loop.entry
    // %loop.exit:       ; break jumps here
    //
    auto entry_label = block->stmt_size() + 1;
    continue_inst.push_back({});
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

    auto continue_label = block->stmt_size() + 1;
    block->add_stmt(new sir_br(continue_label));
    block->add_stmt(new sir_label(continue_label, "loop.continue"));
    for(auto i : continue_inst.back()) {
        i->set_label(continue_label);
    }
    if (node->get_update()) {
        node->get_update()->accept(this);
    }
    block->add_stmt(new sir_br(entry_label));

    auto exit_label = block->stmt_size();
    cond_ir->set_false_label(exit_label);
    block->add_stmt(new sir_label(exit_label, "loop.exit"));
    for(auto i : break_inst.back()) {
        i->set_label(exit_label);
    }

    continue_inst.pop_back();
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

void mir2sir::generate_llvm_ident() {
    auto ident = new DI_named_metadata("llvm.ident");
    ictx.named_metadata.push_back(ident);

    auto version_list = new DI_list(dwarf_status.DI_counter);
    version_list->add(new DI_string(
        "colgm compiler version " __colgm_ver__
    ));
    ident->add(new DI_ref_index(dwarf_status.DI_counter));
    ictx.debug_info.push_back(version_list);

    ++dwarf_status.DI_counter;
}

void mir2sir::generate_llvm_module_flags() {
    auto module_flags = new DI_named_metadata("llvm.module.flags");
    ictx.named_metadata.push_back(module_flags);

    auto dwarf_version = new DI_list(dwarf_status.DI_counter);
    dwarf_version->add(new DI_i32(7));
    dwarf_version->add(new DI_string("Dwarf Version"));
    dwarf_version->add(new DI_i32(4));
    ictx.debug_info.push_back(dwarf_version);
    module_flags->add(new DI_ref_index(dwarf_status.DI_counter));
    ++dwarf_status.DI_counter;

    auto debug_info_version = new DI_list(dwarf_status.DI_counter);
    debug_info_version->add(new DI_i32(2));
    debug_info_version->add(new DI_string("Debug Info Version"));
    debug_info_version->add(new DI_i32(3));
    ictx.debug_info.push_back(debug_info_version);
    module_flags->add(new DI_ref_index(dwarf_status.DI_counter));
    ++dwarf_status.DI_counter;

    auto wchar_size = new DI_list(dwarf_status.DI_counter);
    wchar_size->add(new DI_i32(1));
    wchar_size->add(new DI_string("wchar_size"));
    wchar_size->add(new DI_i32(4));
    ictx.debug_info.push_back(wchar_size);
    module_flags->add(new DI_ref_index(dwarf_status.DI_counter));
    ++dwarf_status.DI_counter;

    auto uwtable = new DI_list(dwarf_status.DI_counter);
    uwtable->add(new DI_i32(7));
    uwtable->add(new DI_string("uwtable"));
    uwtable->add(new DI_i32(1));
    ictx.debug_info.push_back(uwtable);
    module_flags->add(new DI_ref_index(dwarf_status.DI_counter));
    ++dwarf_status.DI_counter;

    auto frame_pointer = new DI_list(dwarf_status.DI_counter);
    frame_pointer->add(new DI_i32(7));
    frame_pointer->add(new DI_string("frame-pointer"));
    frame_pointer->add(new DI_i32(2));
    ictx.debug_info.push_back(frame_pointer);
    module_flags->add(new DI_ref_index(dwarf_status.DI_counter));
    ++dwarf_status.DI_counter;
}

void mir2sir::generate_llvm_dbg_cu() {
    auto llvm_dbg_cu = new DI_named_metadata("llvm.dbg.cu");
    ictx.named_metadata.push_back(llvm_dbg_cu);

    auto main_input_file_index = ictx.DI_file_map.at(ctx.global.input_file);
    dwarf_status.compile_unit_index = dwarf_status.DI_counter;
    auto cu = new DI_compile_unit(
        dwarf_status.DI_counter,
        "colgm compiler version " __colgm_ver__,
        main_input_file_index
    );
    llvm_dbg_cu->add(new DI_ref_index(dwarf_status.DI_counter));
    ictx.debug_info.push_back(cu);
    ++dwarf_status.DI_counter;
}

void mir2sir::generate_DIFile() {
    ictx.DI_file_map.clear();

    // empty file name maybe used for auto-generated functions
    std::string empty_file_name = "";
    ictx.debug_info.push_back(
        new DI_file(dwarf_status.DI_counter, empty_file_name, empty_file_name)
    );
    ictx.DI_file_map.insert({empty_file_name, dwarf_status.DI_counter});
    ++dwarf_status.DI_counter;

    for (const auto& i : ctx.global.domain) {
        const auto& filename = i.first;
        const auto directory = std::string("");
        ictx.debug_info.push_back(
            new DI_file(dwarf_status.DI_counter, filename, directory)
        );
        ictx.DI_file_map.insert({filename, dwarf_status.DI_counter});
        ++dwarf_status.DI_counter;
    }
}

void mir2sir::generate_basic_type() {
    ictx.DI_basic_type_map.clear();
    struct {
        std::string name;
        u64 size_in_bits;
        std::string encoding;
    } type_table[] = {
        {"i8", 8, "DW_ATE_signed"},
        {"i16", 16, "DW_ATE_signed"},
        {"i32", 32, "DW_ATE_signed"},
        {"i64", 64, "DW_ATE_signed"},
        {"u8", 8, "DW_ATE_unsigned"},
        {"u16", 16, "DW_ATE_unsigned"},
        {"u32", 32, "DW_ATE_unsigned"},
        {"u64", 64, "DW_ATE_unsigned"},
        {"f32", 32, "DW_ATE_float"},
        {"f64", 64, "DW_ATE_float"},
        {"bool", 1, "DW_ATE_boolean"}
    };
    for (auto& i : type_table) {
        ictx.debug_info.push_back(
            new DI_basic_type(
                dwarf_status.DI_counter,
                i.name,
                i.size_in_bits,
                i.encoding
            )
        );
        ictx.DI_basic_type_map.insert({i.name, dwarf_status.DI_counter});
        ++dwarf_status.DI_counter;
    }
}

void mir2sir::generate_DI_enum_type(const mir_context&) {
    for (auto& d : ctx.global.domain) {
        for (auto& e : d.second.enums) {
            const auto ty = type {
                .name = e.second.name,
                .loc_file = e.second.location.file
            };
            const auto id = mangle(ty.full_path_name());
            auto tmp = new DI_enum_type(
                dwarf_status.DI_counter,
                e.second.name,
                id,
                ictx.DI_file_map.at(e.second.location.file),
                e.second.location.begin_line,
                ictx.DI_basic_type_map.at("i64")
            );
            ictx.debug_info.push_back(tmp);
            ++dwarf_status.DI_counter;

            auto enumerator_list = new DI_list(dwarf_status.DI_counter);
            ictx.debug_info.push_back(enumerator_list);
            tmp->set_elements_index(dwarf_status.DI_counter);
            ++dwarf_status.DI_counter;

            for (auto& et : e.second.ordered_member) {
                ictx.debug_info.push_back(
                    new DI_enumerator(
                        dwarf_status.DI_counter,
                        et,
                        e.second.members.at(et)
                    )
                );
                enumerator_list->add(new DI_ref_index(dwarf_status.DI_counter));
                ++dwarf_status.DI_counter;
            }
        }
    }
}

void mir2sir::generate_DI_structure_type(const mir_context& mctx) {
    for (auto i : mctx.structs) {
        const auto ty = type {
            .name = i->name,
            .loc_file = i->location.file
        };
        const auto id = "struct." + mangle(ty.full_path_name());
        auto tmp = new DI_structure_type(
            dwarf_status.DI_counter,
            i->name,
            id,
            ictx.DI_file_map.at(i->location.file),
            i->location.begin_line
        );
        ictx.debug_info.push_back(tmp);
        ++dwarf_status.DI_counter;
    }
}

void mir2sir::generate_DI_subprogram(const mir_context& mctx) {
    for (auto& i : mctx.decls) {
        // auto added libc func like free, malloc may not have location
        if (!ictx.DI_file_map.count(i->location.file)) {
            continue;
        }
        auto tmp = new DI_subprogram(
            dwarf_status.DI_counter,
            i->name,
            ictx.DI_file_map.at(i->location.file),
            i->location.begin_line,
            dwarf_status.compile_unit_index
        );
        ictx.debug_info.push_back(tmp);
        ++dwarf_status.DI_counter;
    }
    for (auto& i : mctx.impls) {
        auto tmp = new DI_subprogram(
            dwarf_status.DI_counter,
            i->name,
            ictx.DI_file_map.at(i->location.file),
            i->location.begin_line,
            dwarf_status.compile_unit_index
        );
        dwarf_status.impl_debug_info.insert(
            {i->name, dwarf_status.DI_counter}
        );
        ictx.debug_info.push_back(tmp);
        ++dwarf_status.DI_counter;
    }
}

void mir2sir::generate_DWARF(const mir_context& mctx) {
    dwarf_status.DI_counter = 0;
    dwarf_status.compile_unit_index = DI_node::DI_ERROR_INDEX;
    dwarf_status.impl_debug_info.clear();
    generate_llvm_ident();
    generate_llvm_module_flags();
    generate_DIFile();
    generate_llvm_dbg_cu();
    generate_basic_type();
    generate_DI_enum_type(mctx);
    generate_DI_structure_type(mctx);
    generate_DI_subprogram(mctx);
}

const error& mir2sir::generate(const mir_context& mctx) {
    generate_DWARF(mctx);

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
