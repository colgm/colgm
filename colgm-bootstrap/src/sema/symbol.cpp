#include "colgm.h"
#include "report.h"
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

std::unordered_map<std::string, colgm_basic*> colgm_basic::mapper = {
    {"i32", colgm_basic::i32()},
    {"i64", colgm_basic::i64()}
};

colgm_basic* colgm_basic::i32() {
    static colgm_basic singleton = colgm_basic {"i32", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::i32_type()};
    singleton.static_method.insert({
        "to_i64",
        colgm_func {
            .name = "to_i64",
            .location = span::null(),
            .return_type = type::i64_type(),
            .parameters = {parameter},
            .unordered_params = {{"_num", parameter}}
        }
    });
    return &singleton;
}

colgm_basic* colgm_basic::i64() {
    static colgm_basic singleton = colgm_basic {"i64", {}};
    if (!singleton.static_method.empty()) {
        return &singleton;
    }
    const auto parameter = symbol {.name = "_num", type::i64_type()};
    singleton.static_method.insert({
        "to_i32",
        colgm_func {
            .name = "to_i32",
            .location = span::null(),
            .return_type = type::i32_type(),
            .parameters = {parameter},
            .unordered_params = {{"_num", parameter}}
        }
    });
    return &singleton;
}

}