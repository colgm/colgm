#pragma once

#include "colgm.h"
#include "ast/ast.h"
#include "ast/decl.h"
#include "report.h"

#include <cstring>
#include <sstream>
#include <vector>

namespace colgm::ast {

class delete_disabled_node {
private:
    std::string arch;
    std::string platform;

private:
    bool check_enable_if(error&, cond_compile*);
    bool check_conds(error&, const std::vector<cond_compile*>&);
    void report_not_supported_condition(error&, impl*);

public:
    delete_disabled_node(): arch(get_arch()), platform(get_platform()) {}
    void scan(error&, root*);
};

}