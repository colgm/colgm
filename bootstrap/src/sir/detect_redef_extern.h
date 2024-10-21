#pragma once

#include "report.h"
#include "sir/pass_manager.h"

#include <cstring>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

namespace colgm {

class detect_redef_extern: public sir_pass {
private:
    error err;

public:
    detect_redef_extern(): sir_pass() {}
    ~detect_redef_extern() override = default;
    std::string name() override {
        return "detect redef extern";
    }
    std::string info() override {
        return "passed";
    }
    bool run(sir_context*) override;
};

}
