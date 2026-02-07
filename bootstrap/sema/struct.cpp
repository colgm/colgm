#include "sema/struct.h"

#include <vector>

namespace colgm {

colgm_struct::~colgm_struct() {
    if (generic_struct_decl) {
        delete generic_struct_decl;
    }
    for (auto i : generic_struct_impl) {
        delete i;
    }
}

usize colgm_struct::field_index(const std::string& name) const {
    for (usize i = 0; i < ordered_field.size(); ++i) {
        if (ordered_field[i] == name) {
            return i;
        }
    }
    return SIZE_MAX;
}

std::string colgm_struct::fuzzy_match_field(const std::string& name) const {
    std::string res = "";
    usize min_distance = SIZE_MAX;

    for (const auto& i : field) {
        usize distance = levenshtein_distance(name, i.first);
        if (distance < min_distance) {
            res = i.first;
            min_distance = distance;
        }
    }

    for (const auto& i : method) {
        usize distance = levenshtein_distance(name, i.first);
        if (distance < min_distance) {
            res = i.first;
            min_distance = distance;
        }
    }

    for (const auto& i : static_method) {
        usize distance = levenshtein_distance(name, i.first);
        if (distance < min_distance) {
            res = i.first;
            min_distance = distance;
        }
    }

    return res;
}

}