#pragma once

#include "pass/pass.h"

namespace colgm {

class add_default_main: public pass {
public:
    add_default_main(ir_context& c):
        pass(pass_kind::ps_default_main, c) {}
    ~add_default_main() override = default;
    bool run() override;
};

}