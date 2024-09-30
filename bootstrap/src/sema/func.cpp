#include "sema/func.h"

namespace colgm {

colgm_func::~colgm_func() {
    if (generic_func_decl) {
        delete generic_func_decl;
    }
}

bool colgm_func::find_parameter(const std::string& name) {
    return unordered_params.count(name);
}

void colgm_func::add_parameter(const std::string& name, const type& t) {
    parameters.push_back({name, t});
    unordered_params.insert({name, {name, t}});
}

}