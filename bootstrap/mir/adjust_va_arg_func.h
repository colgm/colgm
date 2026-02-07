#pragma once

#include "mir/pass_manager.h"

#include <sstream>
#include <cstring>
#include <unordered_map>

namespace colgm::mir {

class adjust_va_arg_func: public pass {
private:
    std::unordered_map<std::string, mir_func*> used_funcs;

private:
    void adjust_posix_open();

public:
    ~adjust_va_arg_func() override = default;
    std::string name() override {
        return "adjust va_arg func";
    }
    bool run(mir_context*) override;
};

}