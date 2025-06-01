#include "mir/adjust_va_arg_func.h"

namespace colgm::mir {

void adjust_va_arg_func::adjust_posix_open() {
    if (!used_funcs.count("open")) {
        return;
    }

    auto open_func = used_funcs.at("open");
    while (open_func->params.size() > 2) {
        open_func->params.pop_back();
    }
    open_func->with_va_args = true;
}

bool adjust_va_arg_func::run(mir_context* c) {
    for (auto i : c->decls) {
        used_funcs.insert({i->name, i});
    }
    for (auto i : c->impls) {
        used_funcs.insert({i->name, i});
    }

    adjust_posix_open();
    return true;
}

}