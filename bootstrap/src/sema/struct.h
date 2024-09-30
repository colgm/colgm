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
    std::unordered_map<std::string, symbol> field;
    std::vector<symbol> ordered_field;
    std::unordered_map<std::string, colgm_func> static_method;
    std::unordered_map<std::string, colgm_func> method;

public:
    std::vector<std::string> generic_template;
    ast::impl_struct* generic_struct_impl = nullptr;

public:
    bool is_public = false;
    bool is_extern = false;

public:
    ~colgm_struct();
    usize field_index(const std::string&) const;
};

}