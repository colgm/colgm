#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "colgm.h"
#include "report.h"
#include "ast/ast.h"
#include "ast/decl.h"
#include "sema/func.h"

namespace colgm {

struct colgm_struct {
public:
    std::string name;
    span location;
    std::unordered_map<std::string, type> field;
    std::vector<std::string> ordered_field;
    std::unordered_map<std::string, colgm_func> static_method;
    std::unordered_map<std::string, colgm_func> method;

public:
    std::vector<std::string> generic_template;
    ast::struct_decl* generic_struct_decl = nullptr;
    std::vector<ast::impl_struct*> generic_struct_impl;

public:
    bool is_public = false;
    bool is_extern = false;

public:
    colgm_struct() = default;
    colgm_struct(const colgm_struct& s):
        name(s.name), location(s.location),
        field(s.field), ordered_field(s.ordered_field),
        static_method(s.static_method), method(s.method),
        generic_template(s.generic_template),
        generic_struct_decl(nullptr),
        generic_struct_impl({}), is_public(s.is_public),
        is_extern(s.is_extern) {
        if (s.generic_struct_decl) {
            generic_struct_decl = s.generic_struct_decl->clone();
        }
        for (auto i : s.generic_struct_impl) {
            generic_struct_impl.push_back(i->clone());
        }
    }
    ~colgm_struct();
    usize field_index(const std::string&) const;
};

}