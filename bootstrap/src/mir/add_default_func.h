#pragma once

#include "mir/pass.h"

#include <sstream>
#include <cstring>
#include <unordered_set>

namespace colgm::mir {

class add_default_func: public pass {
private:
    mir_context* ctx = nullptr;
    std::unordered_set<std::string> used_funcs;

private:
    void add_malloc_decl();
    void add_free_decl();
    void add_main_impl();

public:
    std::string name() override {
        return "add_default_func";
    }
    bool run(mir_context*) override;
};

}