#include "colgm.h"
#include "sema/symbol.h"

namespace colgm {

std::string type::to_string() const {
    auto result = name;
    for(u64 i = 0; i<pointer_level; ++i) {
        result += "*";
    }
    return result;
}

bool type::operator==(const type& another) const {
    return name==another.name && pointer_level==another.pointer_level;
}

bool type::operator!=(const type& another) const {
    return name!=another.name || pointer_level!=another.pointer_level;
}

std::ostream& operator<<(std::ostream& out, const type& t) {
    out << t.name;
    for(u64 i = 0; i < t.pointer_level; ++i) {
        out << "*";
    }
    return out;
}

bool colgm_func::find_parameter(const std::string& name) {
    for(const auto& i : parameters) {
        if (i.name==name) {
            return true;
        }
    }
    return false;
}

void colgm_func::add_parameter(const std::string& name, const type& t) {
    parameters.push_back({name, t});
}

}