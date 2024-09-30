#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>


#include "report.h"
#include "symbol.h"
#include "ast/ast.h"
#include "ast/decl.h"

namespace colgm {

struct colgm_func {
public:
    std::string name;
    span location;
    type return_type;
    std::vector<symbol> parameters;
    std::unordered_map<std::string, symbol> unordered_params;

public:
    std::vector<std::string> generic_template;
    ast::func_decl* generic_func_decl = nullptr;

public:
    bool is_public = false;
    bool is_extern = false;

public:
    ~colgm_func();
    bool find_parameter(const std::string&);
    void add_parameter(const std::string&, const type&);
};

}