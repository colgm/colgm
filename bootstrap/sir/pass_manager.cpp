#include "sir/pass_manager.h"
#include "sir/delete_useless_label.h"
#include "sir/remove_alloca.h"
#include "sir/detect_redef_extern.h"
#include "sir/primitive_size_opt.h"
#include "report.h"

namespace colgm {

sir_pass_manager::~sir_pass_manager() {
    for(auto i : passes) {
        delete i;
    }
}

void sir_pass_manager::execute(sir_context* sctx) {
    passes.push_back(new delete_useless_label);
    passes.push_back(new remove_alloca);
    passes.push_back(new detect_redef_extern);
    passes.push_back(new primitive_size_opt);
    for(auto i : passes) {
        std::clog << "[" << green << "sir" << reset;
        std::clog << "] run " << i->name() << " pass...\n";
        if (!i->run(sctx)) {
            break;
        }
        std::clog << "[" << green << "sir" << reset;
        std::clog << "] run " << i->name() << " pass: ";
        std::clog << i->info() << "\n";
    }
}

}