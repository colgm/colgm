#include "sir/pass.h"
#include "sir/pass_manager.h"
#include "sir/add_default_func.h"

namespace colgm {

void pass_manager::run(sir_context& c) {
    ordered_passes.clear();
    ordered_passes.push_back(new add_default_func(c));
    for(auto i : ordered_passes) {
        if (!i->run()) {
            break;
        }
    }
}

}