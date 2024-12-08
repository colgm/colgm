#include "sir/delete_useless_label.h"

#include <unordered_set>

namespace colgm {

void delete_useless_label::add_count(std::unordered_map<std::string, u64>& m,
                                     const std::string& label) {
    if (!m.count(label)) {
        m.insert({label, 1});
    } else {
        m[label]++;
    }
}

void delete_useless_label::delete_label(sir_block* cb) {
    std::unordered_map<std::string, u64> used_label;
    for(auto i : cb->get_stmts()) {
        switch(i->get_ir_type()) {
            case sir_kind::sir_br: {
                auto label = i->to<sir_br>()->get_label();
                add_count(used_label, label);
            } break;
            case sir_kind::sir_br_cond: {
                auto left = i->to<sir_br_cond>()->get_label_true();
                auto right = i->to<sir_br_cond>()->get_label_false();
                add_count(used_label, left);
                add_count(used_label, right);
            } break;
            case sir_kind::sir_switch: {
                auto label = i->to<sir_switch>()->get_default_label();
                add_count(used_label, label);
                for (const auto& j : i->to<sir_switch>()->get_case_labels()) {
                    add_count(used_label, j);
                }
            } break;
            default: break;
        }
    }

    std::unordered_set<sir*> useless_inst;
    for(usize i = 0; i<cb->stmt_size(); ++i) {
        if (i + 1 >= cb->stmt_size()) {
            continue;
        }
        auto this_node = cb->get_stmts()[i];
        auto next_node = cb->get_stmts()[i+1];
        if (this_node->get_ir_type()!=sir_kind::sir_br) {
            continue;
        }
        if (next_node->get_ir_type()!=sir_kind::sir_label &&
            next_node->get_ir_type()!=sir_kind::sir_place_holder_label) {
            continue;
        }

        const auto br_label = this_node->to<sir_br>()->get_label();
        const auto real_label = next_node->get_ir_type()==sir_kind::sir_label ?
                            next_node->to<sir_label>()->get_label() :
                            next_node->to<sir_place_holder_label>()->get_label();
        if (br_label==real_label && used_label.at(br_label)==1) {
            useless_inst.insert(this_node);
            useless_inst.insert(next_node);
        }
    }

    std::vector<sir*> after_remove = {};
    for(auto i : cb->get_stmts()) {
        if (!useless_inst.count(i)) {
            after_remove.push_back(i);
        } else {
            ++removed_count;
            delete i;
        }
    }
    cb->get_mut_stmts() = after_remove;

    for(auto i : cb->get_stmts()) {
        if (i->get_ir_type()==sir_kind::sir_block) {
            delete_label(i->to<sir_block>());
        }
    }
}

bool delete_useless_label::run(sir_context* ctx) {
    for(auto i : ctx->func_impls) {
        delete_label(i->get_code_block());
    }
    return true;
}

}