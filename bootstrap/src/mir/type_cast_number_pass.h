#pragma once

#include "mir/ast2mir.h"
#include "mir/pass_manager.h"

#include <cstring>
#include <sstream>

namespace colgm::mir {

class type_cast_number: public pass {
private:
    mir_number* do_optimize(mir_type_convert*);
    bool cast_const_number(mir_type_convert*);
    void visit_mir_block(mir_block*) override;

public:
    std::string name() override {
        return "constant number type cast optimization";
    }
    bool run(mir_context*) override;
};

}
