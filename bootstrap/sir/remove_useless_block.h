#pragma once

#include "sir/pass_manager.h"

namespace colgm {

class remove_useless_block : public sir_pass {
private:
    u64 remove_count;
    void do_single_remove(sir_func*);

public:
    remove_useless_block(): sir_pass(), remove_count(0) {}
    ~remove_useless_block() override = default;
    std::string name() override {
        return "remove useless block";
    }
    std::string info() override {
        return "remove " +
               std::to_string(remove_count) +
               " useless basic block" +
               (remove_count > 1 ? "s" : "");
    }
    bool run(sir_context*) override;
};

}