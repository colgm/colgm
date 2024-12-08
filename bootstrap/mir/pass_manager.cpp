#include "mir/pass_manager.h"
#include "mir/add_default_func.h"
#include "mir/type_cast_number_pass.h"
#include "report.h"

#include <iostream>

namespace colgm::mir {

pass_manager::~pass_manager() {
    for(auto i : work_list) {
        delete i;
    }
}

void pass_manager::execute(mir_context* mctx, bool verbose) {
    work_list.push_back(new add_default_func);
    work_list.push_back(new type_cast_number);

    for(auto i : work_list) {
        if (verbose) {
            std::clog << "[" << green << "mir" << reset;
            std::clog << "] run " << i->name() << " pass...\n";
        }
        if (!i->run(mctx)) {
            break;
        }
        if (verbose) {
            std::clog << "[" << green << "mir" << reset;
            std::clog << "] run " << i->name() << " pass finished\n";
        }
    }
}

}
