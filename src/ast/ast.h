#pragma once

#include "err.h"

namespace colgm {

enum ast_type {
    ast_null = 0,
    ast_use_stmt
};

class visitor;
class ast_node {
private:
    ast_type node_type;
    span location;

public:
    ast_node(ast_type type, const span& loc): node_type(type), location(loc) {}
    virtual ~ast_node() = default;
    virtual void accept(visitor*) = 0;
};

}