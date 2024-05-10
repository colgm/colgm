#pragma once

#include "code/sir.h"
#include "code/context.h"
#include "report.h"

#include <sstream>
#include <cstring>

namespace colgm {

enum class pass_kind {
    ps_add_default_func,
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
    virtual const std::string& get_name() = 0;
    virtual bool run() = 0;    
};

}