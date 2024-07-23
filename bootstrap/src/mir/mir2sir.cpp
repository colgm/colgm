#include "mir/mir2sir.h"

#include <fstream>

namespace colgm::mir {

void mir2sir::emit_struct(const mir_context& mctx) {
    for(auto i : mctx.structs) {
        auto stct = hir_struct(i->name, i->location);
        for(const auto& f : i->field_type) {
            stct.add_field_type(f.to_string());
        }
        ictx.struct_decls.push_back(stct);
    }
}

void mir2sir::emit_func_decl(const mir_context& mctx) {
    for(auto i : mctx.decls) {
        auto func = new hir_func(i->name);
        func->set_return_type(i->return_type.to_string());
        for(const auto& j : i->params) {
            func->add_param(j.first + ".param", j.second.to_string());
        }
        ictx.func_decls.push_back(func);
    }
}

void mir2sir::emit_func_impl(const mir_context& mctx) {
    for(auto i : mctx.impls) {
        auto func = new hir_func(i->name);
        func->set_return_type(i->return_type.to_string());
        for(const auto& j : i->params) {
            func->add_param(j.first + ".param", j.second.to_string());
        }
        // generate code block
        func->set_code_block(new sir_block);
        block = func->get_code_block();
        for(const auto& j : i->params) {
            block->add_alloca(new sir_alloca(j.first, j.second.to_string()));
        }
        i->block->accept(this);
        block = nullptr;

        ictx.func_impls.push_back(func);
    }
}

void mir2sir::visit_mir_define(mir_define* node) {
    block->add_alloca(new sir_alloca(
        node->get_name(),
        node->get_resolve_type().to_string()
    ));
}

void mir2sir::generate(const mir_context& mctx) {
    for(const auto& i : mctx.const_strings) {
        ictx.const_strings.insert(i);
    }
    emit_struct(mctx);
    emit_func_decl(mctx);
    emit_func_impl(mctx);

    std::ofstream out("test_mir2sir.dump.ll");
    ictx.dump_code(out);
}

}
