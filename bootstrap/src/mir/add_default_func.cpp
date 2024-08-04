#include "mir/add_default_func.h"

namespace colgm::mir {

bool add_default_func::check_used(const std::string& name) {
    for(auto i : ctx->decls) {
        if (i->name==name) {
            return true;
        }
    }
    for(auto i : ctx->impls) {
        if (i->name==name) {
            return true;
        }
    }
    return false;
}

void add_default_func::add_malloc_decl() {
    if (check_used("@malloc")) {
        return;
    }

    auto malloc_decl = new mir_func;
    malloc_decl->name = "@malloc";
    malloc_decl->return_type = type::i8_type(1);
    malloc_decl->params.push_back({"size", type::i64_type()});
    ctx->decls.push_back(malloc_decl);
}

void add_default_func::add_free_decl() {
    if (check_used("@free")) {
        return;
    }

    auto free_decl = new mir_func;
    free_decl->name = "@free";
    free_decl->return_type = type::void_type();
    free_decl->params.push_back({"ptr", type::i8_type(1)});
    ctx->decls.push_back(free_decl);
}

void add_default_func::add_main_func_impl() {
    if (check_used("@main")) {
        return;
    }

    auto default_main = new mir_func;
    default_main->name = "@main";
    default_main->return_type = type::i32_type();
    default_main->block = new mir_block(span::null());
    auto return_stmt = new mir_return(span::null(), new mir_block(span::null()));
    return_stmt->get_value()->add_content(
        new mir_number(span::null(), "0", type::i32_type())
    );

    default_main->block->add_content(return_stmt);
    ctx->impls.push_back(default_main);
}

bool add_default_func::run(mir_context* c) {
    ctx = c;

    add_malloc_decl();
    add_free_decl();
    add_main_func_impl();
    return true;
}

}