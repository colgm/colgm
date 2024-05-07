#pragma once

#include "code/sir.h"
#include "code/context.h"
#include "report.h"

namespace colgm {

enum class pass_kind {
    ps_default_main,
    ps_native_type_conv
};

class pass {
protected:
    pass_kind kind;
    error err;
    ir_context* ctx;

public:
    pass(pass_kind k, ir_context& c): kind(k), ctx(&c) {}
    virtual ~pass() = default;
    virtual bool run() = 0;    
};

}