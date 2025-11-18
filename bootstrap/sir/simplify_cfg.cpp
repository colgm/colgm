#include "sir/control_flow.h"
#include "sir/simplify_cfg.h"

#include <vector>

namespace colgm {

u64 remove_no_pred_block::do_single_remove(sir_func* func) {
    if (func->get_code_block()->get_basic_blocks().size() <= 1) {
        return 0;
    }

    u64 count = 0;
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
        count++;
    }

    if (!count) {
        return 0;
    }

    func_block->get_mutable_basic_blocks() = blocks;
    return count;
}

u64 remove_no_pred_block::iter_do_remove(sir_context* ctx) {
    u64 count = 0;
    for (auto func : ctx->func_impls) {
        count += do_single_remove(func);
    }
    return count;
}

bool remove_no_pred_block::run(sir_context* ctx) {
    auto cfa = control_flow_analysis();
    cfa.run(ctx);
    while (true) {
        auto count = iter_do_remove(ctx);
        if (!count) {
            break;
        }
        cfa.run(ctx);
        remove_count += count;
    }
    return true;
}

}