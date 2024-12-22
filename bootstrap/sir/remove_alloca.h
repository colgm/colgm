#pragma once

#include "sir/pass_manager.h"

#include <cstring>
#include <sstream>

namespace colgm {

class remove_alloca: public sir_pass {
private:
    u64 removed_count;

private:
    void do_remove(sir_block*);

public:
    remove_alloca(): sir_pass(), removed_count(0) {}
    ~remove_alloca() override = default;
    std::string name() override {
        return "remove alloca";
    }
    std::string info() override {
        std::stringstream ss;
        ss << "merged " << removed_count << " instruction";
        if (removed_count > 1) {
            ss << "s";
        }
        return ss.str();
    }
    bool run(sir_context*) override;
};

}