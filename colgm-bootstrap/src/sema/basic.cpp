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

}

std::unordered_map<std::string, colgm_basic*> colgm_basic::mapper = {
    {"i32", colgm_basic::i32()},
    {"i64", colgm_basic::i64()}
};

colgm_basic* colgm_basic::i32() {
    static colgm_basic singleton = colgm_basic {"i32", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::i32_type()};
    singleton.static_method.insert({"to_u8", basic::create_convert_method("to_u8", type::u8_type(), parameter)});
    singleton.static_method.insert({"to_u16", basic::create_convert_method("to_u16", type::u16_type(), parameter)});
    singleton.static_method.insert({"to_u32", basic::create_convert_method("to_u32", type::u32_type(), parameter)});
    singleton.static_method.insert({"to_u64", basic::create_convert_method("to_u64", type::u64_type(), parameter)});
    singleton.static_method.insert({"to_i8", basic::create_convert_method("to_i8", type::i8_type(), parameter)});
    singleton.static_method.insert({"to_i16", basic::create_convert_method("to_i16", type::i16_type(), parameter)});
    singleton.static_method.insert({"to_i64",basic::create_convert_method("to_i64", type::i64_type(), parameter)});
    return &singleton;
}

colgm_basic* colgm_basic::i64() {
    static colgm_basic singleton = colgm_basic {"i64", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::i64_type()};
    singleton.static_method.insert({"to_u8", basic::create_convert_method("to_u8", type::u8_type(), parameter)});
    singleton.static_method.insert({"to_u16", basic::create_convert_method("to_u16", type::u16_type(), parameter)});
    singleton.static_method.insert({"to_u32", basic::create_convert_method("to_u32", type::u32_type(), parameter)});
    singleton.static_method.insert({"to_u64", basic::create_convert_method("to_u64", type::u64_type(), parameter)});
    singleton.static_method.insert({"to_i8", basic::create_convert_method("to_i8", type::i8_type(), parameter)});
    singleton.static_method.insert({"to_i16", basic::create_convert_method("to_i16", type::i16_type(), parameter)});
    singleton.static_method.insert({"to_i32", basic::create_convert_method("to_i32", type::i32_type(), parameter)});
    return &singleton;
}

}