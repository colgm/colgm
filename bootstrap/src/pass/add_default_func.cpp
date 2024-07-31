#include "pass/add_default_func.h"

namespace colgm {

bool add_default_func::check_used(const std::string& name) {
    for(auto i : ctx->func_decls) {
        if (i->get_name()==name) {
            return true;
        }
    }
    for(auto i : ctx->func_impls) {
        if (i->get_name()==name) {
            return true;
        }
    }
    return false;
}

void add_default_func::add_malloc_decl() {
    if (check_used("@malloc")) {
        return;
    }

    auto malloc_decl = new sir_func("@malloc");
    malloc_decl->set_return_type("i8*");
    malloc_decl->add_param("size", "i64");
    ctx->func_decls.push_back(malloc_decl);
}

void add_default_func::add_free_decl() {
    if (check_used("@free")) {
        return;
    }

    auto free_decl = new sir_func("@free");
    free_decl->set_return_type("void");
    free_decl->add_param("ptr", "i8*");
    ctx->func_decls.push_back(free_decl);
}

void add_default_func::add_main_func_impl() {
    if (check_used("@main")) {
        return;
    }

    auto default_main = new sir_func("@main");
    default_main->set_return_type("i32");
    default_main->set_code_block(new sir_block);
    default_main->get_code_block()->add_stmt(
        new sir_ret("i32", value_t::literal("0"))
    );
    ctx->func_impls.push_back(default_main);
}

bool add_default_func::run() {
    add_malloc_decl();
    add_free_decl();
    add_main_func_impl();
    return true;
}

}