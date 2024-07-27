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

std::string mir2sir::mangle_struct(const std::string& name) {
    return "%struct." + mangle_in_module_symbol(name);
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
        case symbol_kind::struct_kind: copy.name = mangle_struct(full_name); break;
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
                node->get_type().full_path_name(),
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
    node->get_index()->accept(this);
    // TODO
}

void mir2sir::visit_mir_call_func(mir_call_func* node) {
    node->get_args()->accept(this);
    // TODO
}

void mir2sir::visit_mir_call_field(mir_call_field* node) {
    ;
}

void mir2sir::visit_mir_call_path(mir_call_path* node) {
    ;
}

void mir2sir::visit_mir_ptr_call_field(mir_ptr_call_field* node) {
    ;
}


void mir2sir::visit_mir_define(mir_define* node) {
    block->add_alloca(new sir_alloca(
        node->get_name(),
        type_mapping(node->get_resolve_type())
    ));
    node->get_init_value()->accept(this);
    // if (!value_stack_check(node->get_init_value()->get_location())) {
    //     return;
    // }
    // TODO
}

void mir2sir::visit_mir_assign(mir_assign* node) {
    in_assign_lvalue_process = true;
    node->get_left()->accept(this);
    in_assign_lvalue_process = false;

    node->get_right()->accept(this);
    // if (!value_stack_check(node->get_value()->get_location())) {
    //     return;
    // }
    // TODO
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
