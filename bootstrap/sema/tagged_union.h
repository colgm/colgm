#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "colgm.h"
#include "report.h"
#include "sema/symbol.h"

namespace colgm {

struct colgm_tagged_union {
public:
    std::string name;
    type ref_enum_type;
    span location;
    std::unordered_map<std::string, symbol> member;
    std::unordered_map<std::string, i64> member_int_map;
    std::vector<symbol> ordered_member;

public:
    bool is_public = false;
    bool is_extern = false;

public:
    colgm_tagged_union() = default;
    colgm_tagged_union(const colgm_tagged_union& t):
        name(t.name), location(t.location),
        member(t.member), member_int_map(t.member_int_map), ordered_member(t.ordered_member),
        is_public(t.is_public), is_extern(t.is_extern) {}
    ~colgm_tagged_union() = default;
};

}