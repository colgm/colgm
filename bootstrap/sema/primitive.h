#pragma once

#include <cstring>
#include <sstream>
#include <vector>

#include "sema/func.h"

namespace colgm {

struct colgm_primitive {
    std::string name;
    std::unordered_map<std::string, colgm_func> static_method;
    std::unordered_map<std::string, colgm_func> method;
};

}