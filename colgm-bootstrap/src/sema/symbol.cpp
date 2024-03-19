#include "colgm.h"
#include "err.h"
#include "sema/symbol.h"

#include <cstring>
#include <sstream>

namespace colgm {

void type::check_pointer_depth() const {
    if (pointer_depth>=0) {
        return;
    }
    auto info = "invalid pointer depth of \"" + name + "\": ";
    info += std::to_string(pointer_depth);
    info += ", type is defined in \"" + loc_file + "\".";
    error().err("type::check_pointer_depth()", info);
    std::exit(-1);
}

std::string type::to_string() const {
    check_pointer_depth();
    auto result = name;
    for(i64 i = 0; i<pointer_depth; ++i) {
        result += "*";
    }
    return result;
}

std::ostream& operator<<(std::ostream& out, const type& t) {
    t.check_pointer_depth();
    out << t.name;
    for(i64 i = 0; i<t.pointer_depth; ++i) {
        out << "*";
    }
    return out;
}

bool colgm_func::find_parameter(const std::string& name) {
    return unordered_params.count(name);
}

void colgm_func::add_parameter(const std::string& name, const type& t) {
    parameters.push_back({name, t});
    unordered_params.insert({name, {name, t}});
}

}