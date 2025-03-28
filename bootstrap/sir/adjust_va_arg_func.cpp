#include "sir/adjust_va_arg_func.h"

namespace colgm {

void adjust_va_arg_func::adjust_posix_open(sir_context* ctx) {
    // On most platforms, open uses 3 registers to pass the arguments,
    // but on macOS aarch64, the third argument is passed on stack.
    // If we use `open(i8*, i32, i32);`, macOS target executable will malfunction.
    // So we change open to open(i8*, i32, ...) on all the platforms
    // to make sure the third argument functioning properly
    for (auto i : ctx->func_impls) {
        for (auto s : i->get_code_block()->get_stmts()) {
            if (s->get_ir_type() != sir_kind::sir_call) {
                continue;
            }
            auto call = s->to<sir_call>();
            if (call->get_name() != "open") {
                continue;
            }
            call->set_with_va_args(true);
            call->set_with_va_args_real_param_size(2);
        }
    }
}

bool adjust_va_arg_func::run(sir_context* ctx) {
    adjust_posix_open(ctx);
    return true;
}

}