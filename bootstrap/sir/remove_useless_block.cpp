#include "sir/remove_useless_block.h"

#include <vector>

namespace colgm {

void remove_useless_block::do_single_remove(sir_func* func) {
    auto func_block = func->get_code_block();
    std::vector<sir_basic_block*> blocks;

    for (auto i : func_block->get_basic_blocks()) {
        if (!i->get_preds().empty()) {
            blocks.push_back(i);
            continue;
        }
        if (i->get_stmts().size() > 1 || i->back_is_ret_stmt()) {
            blocks.push_back(i);
            continue;
        }
        if (i->get_stmts().size() == 1) {
            auto stmt = i->get_stmts()[0];
            if (stmt->get_ir_type() != sir_kind::sir_br &&
                stmt->get_ir_type() != sir_kind::sir_br_cond) {
                blocks.push_back(i);
                continue;
            }
        }
        delete i;
        remove_count++;
    }

    func_block->get_mutable_basic_blocks() = blocks;
}

bool remove_useless_block::run(sir_context* ctx) {
    for (auto func : ctx->func_impls) {
        do_single_remove(func);
    }
    return true;
}

}