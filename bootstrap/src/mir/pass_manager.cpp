#include "mir/pass_manager.h"
#include "mir/type_cast_number_pass.h"

#include <iostream>

namespace colgm::mir {

void pass_manager::execute(mir_context* mctx) {
    work_list.push_back(new type_cast_number());

    for(auto i : work_list) {
        std::clog << "[mir] running " << i->name() << " pass ...\n";
        if (!i->run(mctx)) {
            break;
        }
    }
}

}
