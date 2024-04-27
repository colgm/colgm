#pragma once

#include "code/context.h"
#include "pass/pass.h"

#include <vector>

namespace colgm {

class pass_manager {
private:
    std::vector<pass*> ordered_passes;

public:
    ~pass_manager() {
        for(auto i : ordered_passes) {
            delete i;
        }
    }
    void run(ir_context&);
};

}