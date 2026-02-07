#include "sir/control_flow.h"

namespace colgm {

sir_basic_block* control_flow_analysis::get_label(sir_block* fb, usize label_num) {
    for (auto i : fb->get_basic_blocks()) {
        if (i->get_label_num() == label_num) {
            return i;
        }
    }
    return nullptr;
}

void control_flow_analysis::analyze_basic_block(sir_basic_block* bb, sir_block* fb) {
    for (auto i : bb->get_stmts()) {
        if (i->get_ir_type() == sir_kind::sir_br) {
            auto br = static_cast<sir_br*>(i);
            auto label = br->get_label_num();
            auto label_bb = get_label(fb, label);
            if (label_bb) {
                label_bb->get_preds().push_back(bb);
                bb->get_succs().push_back(label_bb);
            }
        } else if (i->get_ir_type() == sir_kind::sir_br_cond) {
            auto br = static_cast<sir_br_cond*>(i);
            auto label_true = br->get_label_true_num();
            auto label_false = br->get_label_false_num();
            auto label_true_bb = get_label(fb, label_true);
            auto label_false_bb = get_label(fb, label_false);
            if (label_true_bb) {
                label_true_bb->get_preds().push_back(bb);
                bb->get_succs().push_back(label_true_bb);
            }
            if (label_false_bb) {
                label_false_bb->get_preds().push_back(bb);
                bb->get_succs().push_back(label_false_bb);
            }
        } else if (i->get_ir_type() == sir_kind::sir_switch) {
            auto sw = static_cast<sir_switch*>(i);
            for (auto j : sw->get_cases()) {
                auto label = j.second;
                auto label_bb = get_label(fb, label);
                if (label_bb) {
                    label_bb->get_preds().push_back(bb);
                    bb->get_succs().push_back(label_bb);
                }
            }

            auto default_bb = get_label(fb, sw->get_default_label_num());
            if (default_bb) {
                default_bb->get_preds().push_back(bb);
                bb->get_succs().push_back(default_bb);
            }
        }
    }
}

bool control_flow_analysis::run(sir_context* ctx) {
    for (auto& func : ctx->func_impls) {
        for (auto bb : func->get_code_block()->get_basic_blocks()) {
            bb->get_preds().clear();
            bb->get_succs().clear();
        }
    }
    for (auto& func : ctx->func_impls) {
        for (auto bb : func->get_code_block()->get_basic_blocks()) {
            analyze_basic_block(bb, func->get_code_block());
        }
    }
    return true;
}

}