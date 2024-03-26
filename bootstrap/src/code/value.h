#pragma once

#include "sema/symbol.h"

#include <cstring>
#include <sstream>

namespace colgm {

enum value_kind {
    v_null,
    v_var,
    v_fn,
    v_sfn
};

struct value {
    value_kind kind;
    type resolve_type;
    std::string content;
};

}