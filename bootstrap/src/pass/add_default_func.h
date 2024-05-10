#pragma once

#include "pass/pass.h"

namespace colgm {

class add_default_func: public pass {
private:
    bool check_used(const std::string&);
    void add_malloc_decl();
    void add_free_decl();
    void add_main_func_impl();

public:
    add_default_func(ir_context& c):
        pass(pass_kind::ps_add_default_func, c) {}
    ~add_default_func() override = default;
    const std::string& get_name() override {
        static std::string name = "add_default_func";
        return name;
    }
    bool run() override;
};

}