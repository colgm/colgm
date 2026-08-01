#pragma once

#include "colgm.h"

#include <iostream>
#include <string>

namespace colgm {

struct span {
    u32 begin_line;
    u32 begin_column;
    u32 end_line;
    u32 end_column;
    std::string file;

    static const span null() {
        return {0, 0, 0, 0, ""};
    }
};

std::ostream& operator<<(std::ostream&, const span&);

}
