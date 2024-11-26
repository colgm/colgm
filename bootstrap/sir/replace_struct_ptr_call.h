#pragma once

#include "sir/pass_manager.h"

#include <cstring>
#include <sstream>

namespace colgm {

class replace_struct_ptr_call: public sir_pass {
private:
    u64 replace_count;

private:
    void do_remove(sir_block*);

public:
    replace_struct_ptr_call(): sir_pass(), replace_count(0) {}
    ~replace_struct_ptr_call() override = default;
    std::string name() override {
        return "replace struct __ptr__ call";
    }
    std::string info() override {
        return "replace " +
               std::to_string(replace_count) +
               " __ptr__ method call" +
               (replace_count > 1 ? "s" : "");
    }
    bool run(sir_context*) override;
};

}