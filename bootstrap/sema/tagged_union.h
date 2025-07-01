#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "colgm.h"
#include "report.h"
#include "sema/type.h"
#include "sema/func.h"

namespace colgm {

struct colgm_tagged_union {
public:
    std::string name;
    type ref_enum_type;
    span location;
    std::unordered_map<std::string, type> member;
    std::unordered_map<std::string, i64> member_int_map;
    std::vector<std::string> ordered_member;

    std::unordered_map<std::string, colgm_func> method;
    std::unordered_map<std::string, colgm_func> static_method;

public:
    bool is_public = false;
    bool is_extern = false;

public:
    colgm_tagged_union() = default;
    colgm_tagged_union(const colgm_tagged_union& t):
        name(t.name), location(t.location),
        member(t.member), member_int_map(t.member_int_map), ordered_member(t.ordered_member),
        method(t.method), static_method(t.static_method),
        is_public(t.is_public), is_extern(t.is_extern) {}
    ~colgm_tagged_union() = default;
    std::string fuzzy_match_field(const std::string&) const;
};

}