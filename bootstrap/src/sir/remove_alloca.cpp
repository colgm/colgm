#include "sir/sir.h"
#include "sir/remove_alloca.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace colgm {

void remove_alloca::do_remove(sir_block* node) {
    std::unordered_map<std::string, std::string> register_map = {};
    std::unordered_set<sir_alloca*> to_remove = {};
    for(auto i : node->get_move_register()) {
        if (register_map.count(i->get_type_name())) {
            to_remove.insert(i);
            continue;
        }
        register_map[i->get_type_name()] = i->get_variable_name();
    }

    std::vector<sir_alloca*> after_remove = {};
    for(auto i : node->get_move_register()) {
        if (to_remove.count(i)) {
            ++removed_count;
            delete i;
        } else {
            after_remove.push_back(i);
        }
    }

    node->get_mut_move_register() = after_remove;
    for(auto i : node->get_stmts()) {
        if (i->get_ir_type()!=sir_kind::sir_temp_ptr) {
            continue;
        }
        auto ptr = reinterpret_cast<sir_temp_ptr*>(i);
        ptr->set_source(register_map.at(ptr->get_type()));
    }
}

bool remove_alloca::run(sir_context* ctx) {
    for(auto i : ctx->func_impls) {
        do_remove(i->get_code_block());
    }
    return true;
}

}