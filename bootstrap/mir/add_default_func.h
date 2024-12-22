#pragma once

#include "mir/pass_manager.h"

#include <sstream>
#include <cstring>
#include <unordered_map>

namespace colgm::mir {

class add_default_func: public pass {
private:
    mir_context* ctx = nullptr;
    std::unordered_map<std::string, mir_func*> used_funcs;

private:
    void add_malloc_decl();
    void add_free_decl();
    void add_main_impl();
    void adjust_posix_open();

public:
    ~add_default_func() override = default;
    std::string name() override {
        return "add default function";
    }
    bool run(mir_context*) override;
};

}