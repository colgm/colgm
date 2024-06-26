#include "pass/pass.h"
#include "pass/pass_manager.h"

#include "pass/add_default_func.h"
#include "pass/native_type_convert.h"

namespace colgm {

void pass_manager::run(ir_context& c) {
    ordered_passes.clear();
    ordered_passes.push_back(new native_type_convert(c));
    ordered_passes.push_back(new add_default_func(c));
    for(auto i : ordered_passes) {
        if (!i->run()) {
            break;
        }
    }
}

}