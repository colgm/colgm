#include "sir/replace_struct_ptr_call.h"

#include <vector>

namespace colgm {

void replace_struct_ptr_call::do_remove(sir_block* b) {
    std::vector<sir*> new_stmts = {};
    for(auto i : b->get_stmts()) {
        if (i->get_ir_type() != sir_kind::sir_call) {
            new_stmts.push_back(i);
            continue;
        }
        auto p = static_cast<sir_call*>(i);
        if (p->get_name().find(".__ptr__") != std::string::npos &&
            p->get_args().size() == 1 &&
            p->get_return_type().length() &&
            p->get_return_type().back() == '*') {
            auto rtt = p->get_return_type();
            rtt = rtt.substr(0, rtt.size() - 1);
            auto constant = new sir_temp_ptr(
                p->get_destination().content,
                p->get_args()[0].content,
                rtt,
                rtt + ".__ptr__ call"
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

bool replace_struct_ptr_call::run(sir_context* ctx) {
    for(auto i : ctx->func_impls) {
        do_remove(i->get_code_block());
    }
    return true;
}

}