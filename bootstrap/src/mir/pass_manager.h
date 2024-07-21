#pragma once

#include "mir/ast2mir.h"
#include "mir/pass.h"

#include <vector>

namespace colgm::mir {

class pass_manager {
private:
    std::vector<pass*> work_list;

public:
    void execute(mir_context*);
};

}
