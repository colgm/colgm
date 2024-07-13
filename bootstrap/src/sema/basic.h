#pragma once

#include "sema/symbol.h"

#include <cstring>
#include <sstream>
#include <unordered_map>

namespace colgm {

struct colgm_basic {
    std::string name;
    std::unordered_map<std::string, colgm_func> method;
    std::unordered_map<std::string, colgm_func> static_method;

    static std::unordered_map<std::string, colgm_basic*> mapper;

    static colgm_basic* u8();
    static colgm_basic* u16();
    static colgm_basic* u32();
    static colgm_basic* u64();
    static colgm_basic* i8();
    static colgm_basic* i16();
    static colgm_basic* i32();
    static colgm_basic* i64();
    static colgm_basic* f32();
    static colgm_basic* f64();
};

}