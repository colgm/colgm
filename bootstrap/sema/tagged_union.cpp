#include "sema/tagged_union.h"

namespace colgm {

std::string colgm_tagged_union::fuzzy_match_field(const std::string& name) const {
    std::string res = "";
    usize min_distance = SIZE_MAX;

    for (const auto& i : member) {
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