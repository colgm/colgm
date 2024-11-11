#pragma once

#include "ast/ast.h"
#include "ast/decl.h"

namespace colgm::ast {

class delete_disabled_node {
private:
    bool check_enable_if(cond_compile*);

public:
    void scan(root*);
};

}