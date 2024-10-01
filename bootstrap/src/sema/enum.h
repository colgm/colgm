#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "report.h"

namespace colgm {

struct colgm_enum {
public:
    std::string name;
    span location;
    std::vector<std::string> ordered_member;
    std::unordered_map<std::string, size_t> members;

public:
    bool is_public = false;
};

}