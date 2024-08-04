#pragma once

#include "mir/pass.h"

namespace colgm::mir {

class add_default_func: public pass {
private:
    mir_context* ctx = nullptr;

private:
    bool check_used(const std::string&);
    void add_malloc_decl();
    void add_free_decl();
    void add_main_func_impl();

public:
    std::string name() override {
        return "add_default_func";
    }
    bool run(mir_context*) override;
};

}