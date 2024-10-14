#include "colgm.h"
#include "report.h"
#include "sema/symbol.h"
#include "package/package.h"
#include "ast/decl.h"

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
    for(i64 i = 0; i<pointer_depth; ++i) {
        result += "*";
    }
    return result;
}

std::string type::full_path_name() const {
    const auto pkg = package_manager::singleton();
    const auto module_name = pkg->get_module_name(loc_file);
    if (module_name.empty()) {
        return generic_name();
    }
    return module_name + "::" + generic_name();
}

std::string type::generic_name() const {
    auto result = name;
    if (generics.empty()) {
        return result;
    }
    result += "<";
    for(const auto& g: generics) {
        result += g.full_path_name();
        result += ",";
    }
    if (result.back()==',') {
        result.pop_back();
    }
    result += ">";
    return result;
}

std::ostream& operator<<(std::ostream& out, const type& t) {
    t.check_pointer_depth();
    out << t.full_path_name();
    for(i64 i = 0; i<t.pointer_depth; ++i) {
        out << "*";
    }
    return out;
}

}