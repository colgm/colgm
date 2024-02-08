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

std::ostream& operator<<(std::ostream& out, const type& t) {
    out << t.name;
    for(u64 i = 0; i < t.pointer_level; ++i) {
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