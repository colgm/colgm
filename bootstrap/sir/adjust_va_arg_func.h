#pragma once

#include "report.h"
#include "sir/pass_manager.h"

#include <cstring>
#include <sstream>

namespace colgm {

class adjust_va_arg_func: public sir_pass {
private:
    error err;

private:
    void adjust(sir_call*);
    void adjust_basic_block(sir_basic_block*);
    void adjust_posix_open(sir_context*);

public:
    adjust_va_arg_func(): sir_pass() {}
    ~adjust_va_arg_func() override = default;
    std::string name() override {
        return "adjust va_arg func";
    }
    std::string info() override {
        return "passed";
    }
    bool run(sir_context*) override;
};

}