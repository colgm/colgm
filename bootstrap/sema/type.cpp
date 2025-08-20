#include "colgm.h"
#include "report.h"
#include "sema/type.h"
#include "package/package.h"
#include "ast/decl.h"

#include <iostream>
#include <cstdint>
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
    error().err("type::check_pointer_depth(): " + info);
    std::exit(-1);
}

std::string type::to_string() const {
    check_pointer_depth();
    auto result = generic_name();
    for (i64 i = 0; i < pointer_depth; ++i) {
        result += "*";
    }
    return result;
}

std::string type::array_type_to_string() const {
    if (!is_array) {
        return to_string();
    }
    check_pointer_depth();
    auto result = generic_name();
    for (i64 i = 0; i < pointer_depth - 1; ++i) {
        result += "*";
    }
    return "[" + std::to_string(array_length) + " x " + result + "]";
}

std::string type::full_path_name() const {
    const auto pkg = package_manager::singleton();
    const auto module_name = pkg->get_module_name(loc_file);
    if (module_name.empty()) {
        return generic_name();
    }
    return module_name + "::" + generic_name();
}

std::string type::full_path_name_with_pointer() const {
    auto result = full_path_name();
    for (i64 i = 0; i < pointer_depth; ++i) {
        result += "*";
    }
    return result;
}

std::string type::generic_name() const {
    if (generics.empty()) {
        return name;
    }
    auto result = name + "<";
    for (const auto& g : generics) {
        // should use full path name with pointer info
        // for generic arguments
        // also, full path name will generate generic arguments recursively
        result += g.full_path_name_with_pointer();
        result += ",";
    }
    if (result.back() == ',') {
        result.pop_back();
    }
    result += ">";
    return result;
}

u64 type::generic_depth() const {
    if (generics.empty()) {
        return 0;
    }
    u64 result = 1;
    u64 max_depth = 0;
    for (const auto& g: generics) {
        auto depth = g.generic_depth();
        if (depth > max_depth) {
            max_depth = depth;
        }
    }
    return result + max_depth;
}

std::ostream& operator<<(std::ostream& out, const type& t) {
    t.check_pointer_depth();
    out << t.full_path_name();
    for (i64 i = 0; i < t.pointer_depth; ++i) {
        out << "*";
    }
    return out;
}

void type::dump(const std::string& end) const {
    std::cerr << "(" << name;
    if (!generics.empty()) {
        std::cerr << ":<";
        for (const auto& g: generics) {
            g.dump("");
        }
        std::cerr << ">";
    }
    std::cerr << ")" << end;
}

}