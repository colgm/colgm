#include "sir/pass_manager.h"
#include "sir/delete_useless_label.h"
#include "sir/remove_alloca.h"
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
    for(auto i : passes) {
        std::clog << "[" << green << "sir" << reset;
        std::clog << "] run " << i->name() << " pass...\r";
        if (!i->run(sctx)) {
            break;
        }
        std::clog << "[" << green << "sir" << reset;
        std::clog << "] run " << i->name() << " pass: ";
        std::clog << i->info() << "\n";
    }
}

}