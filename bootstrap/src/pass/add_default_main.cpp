#include "pass/add_default_main.h"

namespace colgm {

bool add_default_main::run() {
    for(auto i : ctx->func_decls) {
        if (i->get_name()=="@main") {
            return true;
        }
    }
    for(auto i : ctx->func_impls) {
        if (i->get_name()=="@main") {
            return true;
        }
    }

    auto default_main = new hir_func("@main");
    default_main->set_return_type("i32");
    default_main->set_code_block(new sir_block);
    default_main->get_code_block()->add_stmt(new sir_ret_literal("i32", "0"));
    ctx->func_impls.push_back(default_main);
    return true;
}

}