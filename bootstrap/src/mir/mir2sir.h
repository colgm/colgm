#pragma once

#include "mir/visitor.h"
#include "mir/ast2mir.h"
#include "code/sir.h"
#include "code/context.h"

namespace colgm::mir {

class mir2sir: public visitor {
private:
    ir_context ictx;

public:
    void generate(const mir_context&);
};

}
