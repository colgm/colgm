#pragma once

#include "pass/pass.h"

namespace colgm {

class native_type_convert: public pass {
public:
    native_type_convert(ir_context& c):
        pass(pass_kind::ps_native_type_conv, c) {}
    ~native_type_convert() override = default;
    const std::string& get_name() override {
        static std::string name = "native_type_convert";
        return name;
    }
    bool run() override;
};

}