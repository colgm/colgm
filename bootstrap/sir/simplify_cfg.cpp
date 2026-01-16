#include "sir/control_flow.h"
#include "sir/simplify_cfg.h"

#include <vector>
#include <unordered_set>

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

u64 merge_block_with_no_cond_br::do_single_merge(sir_func* func) {
    u64 count = 0;
    auto func_block = func->get_code_block();

    std::vector<sir_basic_block*> blocks;
    std::vector<sir_basic_block*> deleted;
    std::unordered_set<u64> locked_block;

    for (auto bb : func_block->get_basic_blocks()) {
        if (bb->get_succs().size() != 1 || bb->get_stmts().size() != 1) {
            blocks.push_back(bb);
            continue;
        }

        if (locked_block.count(bb->get_label_num())) {
            blocks.push_back(bb);
            continue;
        }

        auto succ = bb->get_succs()[0];
        locked_block.insert(succ->get_label_num());

        for (auto pred : bb->get_preds()) {
            auto stmt = pred->get_stmts().back();
            locked_block.insert(pred->get_label_num());
            if (stmt->get_ir_type() == sir_kind::sir_br) {
                auto n = static_cast<sir_br*>(stmt);
                if (n->get_label_num() == bb->get_label_num()) {
                    n->set_label(succ->get_label_num());
                }
            } else if (stmt->get_ir_type() == sir_kind::sir_br_cond) {
                auto n = static_cast<sir_br_cond*>(stmt);
                if (n->get_label_true_num() == bb->get_label_num()) {
                    n->set_true_label(succ->get_label_num());
                }
                if (n->get_label_false_num() == bb->get_label_num()) {
                    n->set_false_label(succ->get_label_num());
                }
            } else if (stmt->get_ir_type() == sir_kind::sir_switch) {
                auto n = static_cast<sir_switch*>(stmt);
                if (n->get_default_label_num() == bb->get_label_num()) {
                    n->set_default_label(succ->get_label_num());
                }

                for (auto& c : n->get_mut_cases()) {
                    if (c.second == bb->get_label_num()) {
                        c.second = succ->get_label_num();
                    }
                }
            }
        }

        delete bb;
        count++;
    }

    if (!count) {
        return 0;
    }

    func_block->get_mutable_basic_blocks() = blocks;
    return count;
}

u64 merge_block_with_no_cond_br::iter_do_merge(sir_context* ctx) {
    u64 count = 0;
    for (auto func : ctx->func_impls) {
        count += do_single_merge(func);
    }
    return count;
}

bool merge_block_with_no_cond_br::run(sir_context* ctx) {
    auto cfa = control_flow_analysis();
    cfa.run(ctx);
    while (true) {
        auto count = iter_do_merge(ctx);
        if (!count) {
            break;
        }
        cfa.run(ctx);
        remove_count += count;
    }
    return true;
}

}