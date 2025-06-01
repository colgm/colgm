#include "sir/primitive_size_opt.h"

namespace colgm {

void primitive_size_opt::remove_primitive_size_method(sir_block* b) {
    std::vector<sir*> new_stmts = {};
    for (auto i : b->get_stmts()) {
        if (i->get_ir_type() != sir_kind::sir_call) {
            new_stmts.push_back(i);
            continue;
        }
        auto p = static_cast<sir_call*>(i);
        if (primitive_methods.count(p->get_name())) {
            auto constant = new sir_add(
                value_t::literal(primitive_methods.at(p->get_name())),
                value_t::literal("0"),
                p->get_destination(),
                true,
                "i64"
            );
            new_stmts.push_back(constant);
            delete p;
            ++replace_count;
        } else {
            new_stmts.push_back(i);
        }
    }
    b->get_mut_stmts() = new_stmts;
}

bool primitive_size_opt::run(sir_context* ctx) {
    for (auto i : ctx->func_impls) {
        remove_primitive_size_method(i->get_code_block());
    }
    return true;
}

}