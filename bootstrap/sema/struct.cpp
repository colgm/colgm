#include "sema/struct.h"

namespace colgm {

colgm_struct::~colgm_struct() {
    if (generic_struct_decl) {
        delete generic_struct_decl;
    }
    for(auto i : generic_struct_impl) {
        delete i;
    }
}

usize colgm_struct::field_index(const std::string& name) const {
    for(usize i = 0; i < ordered_field.size(); ++i) {
        if (ordered_field[i] == name) {
            return i;
        }
    }
    return SIZE_MAX;
}

}