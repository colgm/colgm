#pragma once

#include "sema/symbol.h"

#include <cstring>
#include <sstream>
#include <unordered_map>

namespace colgm {

namespace basic {

colgm_func create_convert_method(const std::string&, const type&, const symbol&);

}

struct colgm_basic {
    std::string name;
    std::unordered_map<std::string, colgm_func> static_method;

    static std::unordered_map<std::string, colgm_basic*> mapper;

    static colgm_basic* i32();
    static colgm_basic* i64();
};

}