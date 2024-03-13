#include "code/context.h"

#include <iomanip>

namespace colgm {

void ir_context::dump_raw_string(std::ostream& out,
                                 const std::string& src) const {
    for(const auto c : src) {
        if (32 <= c && c <= 126) {
            out << c;
        } else {
            out << "\\";
            out << std::hex << std::setw(2) << std::setfill('0') << u32(c);
            out << std::dec;
        }
    }
    out << "\\00";
}

void ir_context::dump_const_string(std::ostream& out) const {
    for(const auto& i : const_strings) {
        out << "@.constant.str." << i.second;
        out << " = private unnamed_addr constant [";
        out << i.first.length() + 1 << " x i8] c\"";
        dump_raw_string(out, i.first);
        out << "\"\n";
    }
}

void ir_context::dump_code(std::ostream& out) const {
    for(const auto& i : struct_decls) {
        i.dump(out);
    }
    dump_const_string(out);
    if (struct_decls.size()) {
        out << "\n";
    }
    for(auto i : func_decls) {
        i->dump(out);
    }
    if (func_decls.size()) {
        out << "\n";
    }
    for(auto i : func_impls) {
        i->dump(out);
        out << "\n";
    }
}

}