#include "sema/symbol.h"

namespace colgm {

std::string type::to_string() const {
    auto result = name;
    for(uint64_t i = 0; i<pointer_level; ++i) {
        result += "*";
    }
    return result;
}

std::ostream& operator<<(std::ostream& out, const type& t) {
    out << t.name;
    for(uint64_t i = 0; i < t.pointer_level; ++i) {
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

bool semantic_context::find_symbol(const std::string& name) {
    for(auto i = scope.rbegin(); i!=scope.rend(); ++i) {
        if (i->count(name)) {
            return true;
        }
    }
    return false;
}

void semantic_context::add_symbol(const std::string& name, const type& t) {
    scope.back().insert({name, t});
}

}