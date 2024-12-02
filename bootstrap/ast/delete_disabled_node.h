#pragma once

#include "colgm.h"
#include "ast/ast.h"
#include "ast/decl.h"

#include <cstring>
#include <sstream>

namespace colgm::ast {

class delete_disabled_node {
private:
    std::string arch;
    std::string platform;

private:
    bool check_enable_if(cond_compile*);

public:
    delete_disabled_node(): arch(get_arch()), platform(get_platform()) {}
    void scan(root*);
};

}