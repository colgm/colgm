#pragma once

#include "mir/mir.h"
#include "mir/visitor.h"
#include "mir/ast2mir.h"

#include <cstring>
#include <sstream>

namespace colgm::mir {

class pass: public visitor {
public:
    virtual std::string name() = 0;
    virtual bool run(mir_context*) = 0;
};

}
