#pragma once

#include "pass/pass.h"

#include <cstring>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

namespace colgm {

class native_type_convert: public pass {
private:
    static inline const std::unordered_map<std::string, std::string> type_mapper = {
        {"u8", "i8"}, {"u16", "i16"},
        {"u32", "i32"}, {"u64", "i64"},
        {"i8", "i8"}, {"i16", "i16"},
        {"i32", "i32"}, {"i64", "i64"},
        {"f32", "float"}, {"f64", "double"}
    };

private:
    void gen(const std::string&, const std::unordered_set<std::string>&);

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