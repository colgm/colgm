#pragma once

#include "sir/pass_manager.h"

namespace colgm {

class control_flow_analysis: public sir_pass {
private:
    sir_basic_block* get_label(sir_block*, usize);
    void analyze_basic_block(sir_basic_block*, sir_block*);

public:
    control_flow_analysis(): sir_pass() {}
    ~control_flow_analysis() override = default;
    std::string name() override {
        return "control flow analysis";
    }
    std::string info() override {
        return "passed";
    }
    bool run(sir_context*) override;
};

}