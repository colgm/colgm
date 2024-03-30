#include "sema/basic.h"

namespace colgm {

namespace basic {

colgm_func create_convert_method(const std::string& function_name,
                                       const type& return_type,
                                       const symbol& self) {
    return colgm_func {
        .name = function_name,
        .location = span::null(),
        .return_type = return_type,
        .parameters = {self},
        .unordered_params = {{self.name, self}}
    };
}

void init_convert_method_map(std::unordered_map<std::string, colgm_func>& m,
                             const symbol& param,
                             const std::string& self) {
    const char* basic_types[] = {
        "u8", "u16", "u32", "u64",
        "i8", "i16", "i32", "i64",
        "f32", "f64"
    };
    for(const auto n : basic_types) {
        if (self==n) {
            continue;
        }
        m.insert({n, create_convert_method(n, type {n, "", 0}, param)});
    }
}

}

std::unordered_map<std::string, colgm_basic*> colgm_basic::mapper = {
    {"u8", colgm_basic::u8()},
    {"u16", colgm_basic::u16()},
    {"u32", colgm_basic::u32()},
    {"u64", colgm_basic::u64()},
    {"i8", colgm_basic::i8()},
    {"i16", colgm_basic::i16()},
    {"i32", colgm_basic::i32()},
    {"i64", colgm_basic::i64()},
    {"f32", colgm_basic::f32()},
    {"f64", colgm_basic::f64()}
};

colgm_basic* colgm_basic::u8() {
    static colgm_basic singleton = colgm_basic {"u8", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::u8_type()};
    basic::init_convert_method_map(singleton.static_method, parameter, "u8");
    return &singleton;
}

colgm_basic* colgm_basic::u16() {
    static colgm_basic singleton = colgm_basic {"u16", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::u16_type()};
    basic::init_convert_method_map(singleton.static_method, parameter, "u16");
    return &singleton;
}

colgm_basic* colgm_basic::u32() {
    static colgm_basic singleton = colgm_basic {"u32", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::u32_type()};
    basic::init_convert_method_map(singleton.static_method, parameter, "u32");
    return &singleton;
}

colgm_basic* colgm_basic::u64() {
    static colgm_basic singleton = colgm_basic {"u64", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::u64_type()};
    basic::init_convert_method_map(singleton.static_method, parameter, "u64");
    return &singleton;
}

colgm_basic* colgm_basic::i8() {
    static colgm_basic singleton = colgm_basic {"i8", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::i8_type()};
    basic::init_convert_method_map(singleton.static_method, parameter, "i8");
    return &singleton;
}

colgm_basic* colgm_basic::i16() {
    static colgm_basic singleton = colgm_basic {"i16", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::i16_type()};
    basic::init_convert_method_map(singleton.static_method, parameter, "i16");
    return &singleton;
}

colgm_basic* colgm_basic::i32() {
    static colgm_basic singleton = colgm_basic {"i32", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::i32_type()};
    basic::init_convert_method_map(singleton.static_method, parameter, "i32");
    return &singleton;
}

colgm_basic* colgm_basic::i64() {
    static colgm_basic singleton = colgm_basic {"i64", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::i64_type()};
    basic::init_convert_method_map(singleton.static_method, parameter, "i64");
    return &singleton;
}

colgm_basic* colgm_basic::f32() {
    static colgm_basic singleton = colgm_basic {"f32", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::f32_type()};
    basic::init_convert_method_map(singleton.static_method, parameter, "f32");
    return &singleton;
}

colgm_basic* colgm_basic::f64() {
    static colgm_basic singleton = colgm_basic {"f64", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::f64_type()};
    basic::init_convert_method_map(singleton.static_method, parameter, "f64");
    return &singleton;
}

}