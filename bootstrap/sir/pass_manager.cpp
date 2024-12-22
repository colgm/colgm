#include "sir/pass_manager.h"
#include "sir/adjust_va_arg_func.h"
#include "sir/delete_useless_label.h"
#include "sir/remove_alloca.h"
#include "sir/detect_redef_extern.h"
#include "sir/primitive_size_opt.h"
#include "sir/replace_struct_ptr_call.h"
#include "report.h"

namespace colgm {

sir_pass_manager::~sir_pass_manager() {
    for(auto i : passes) {
        delete i;
    }
}

void sir_pass_manager::execute(sir_context* sctx, bool verbose) {
    passes.push_back(new adjust_va_arg_func);
    passes.push_back(new delete_useless_label);
    passes.push_back(new remove_alloca);
    passes.push_back(new detect_redef_extern);
    passes.push_back(new primitive_size_opt);
    passes.push_back(new replace_struct_ptr_call);
    for(auto i : passes) {
        if (verbose) {
            std::clog << "[" << green << "sir" << reset;
            std::clog << "] run " << i->name() << " pass...\n";
        }
        if (!i->run(sctx)) {
            break;
        }
        if (verbose) {
            std::clog << "[" << green << "sir" << reset;
            std::clog << "] run " << i->name() << " pass: ";
            std::clog << i->info() << "\n";
        }
    }
}

}