#include "sir/pass_manager.h"
#include "sir/delete_useless_label.h"

namespace colgm {

void sir_pass_manager::execute(sir_context* sctx) {
    passes.push_back(new delete_useless_label);
    for(auto i : passes) {
        std::clog << "[sir] run " << i->name() << " pass...\n";
        if (!i->run(sctx)) {
            break;
        }
        std::clog << "[sir] run " << i->name() << " pass: ";
        std::clog << i->info() << "\n";
        std::clog << "[sir] run " << i->name() << " finished.\n";
    }
}

}