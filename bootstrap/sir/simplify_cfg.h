#pragma once

#include "sir/pass_manager.h"

namespace colgm {

class remove_no_pred_block : public sir_pass {
private:
    u64 remove_count;
    u64 do_single_remove(sir_func*);
    u64 iter_do_remove(sir_context*);

public:
    remove_no_pred_block(): sir_pass(), remove_count(0) {}
    ~remove_no_pred_block() override = default;
    std::string name() override {
        return "remove no pred block";
    }
    std::string info() override {
        return "remove " +
               std::to_string(remove_count) +
               " basic block" +
               (remove_count > 1 ? "s" : "");
    }
    bool run(sir_context*) override;
};

}