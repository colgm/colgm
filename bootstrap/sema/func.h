#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>


#include "report.h"
#include "type.h"
#include "ast/ast.h"
#include "ast/decl.h"

namespace colgm {

struct colgm_func {
public:
    std::string name;
    span location;
    type return_type;
    std::vector<std::string> ordered_params;
    std::unordered_map<std::string, type> params;

public:
    std::vector<std::string> generic_template;
    ast::func_decl* generic_func_decl = nullptr;

public:
    bool is_public = false;
    bool is_extern = false;

public:
    colgm_func() = default;
    colgm_func(const colgm_func& f):
        name(f.name), location(f.location), return_type(f.return_type),
        ordered_params(f.ordered_params), params(f.params),
        generic_template(f.generic_template),
        generic_func_decl(nullptr),
        is_public(f.is_public), is_extern(f.is_extern) {
        if (f.generic_func_decl) {
            generic_func_decl = f.generic_func_decl->clone();
        }
    }
    ~colgm_func();
    bool find_parameter(const std::string&);
    void add_parameter(const std::string&, const type&);
};

}