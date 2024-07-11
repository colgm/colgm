#pragma once

#include "sema/symbol.h"

#include <cstring>
#include <sstream>

namespace colgm {

enum value_kind {
    v_null,
    v_var,
    v_fn, // normal function
    v_static_fn, // static function
    v_type_convert
};

struct value {
    value_kind kind;
    type resolve_type;
    std::string content;

    void dump(std::ostream&) const;
};

}