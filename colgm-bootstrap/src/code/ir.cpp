#include "code/ir.h"

namespace colgm {

void ir_struct::dump(std::ostream& out) {
    out << "%struct." << name << " = type {";
    for(size_t i = 0; i<field_type.size(); ++i) {
        out << field_type[i];
        if (i!=field_type.size()-1) {
            out << ", ";
        }
    }
    out << "}\n";
}

void ir_func_decl::dump(std::ostream& out) {
    out << "define " << return_type << " " << name << "(";
    for(const auto& i : params) {
        out << i.second << " %" << i.first;
        if (i.first!=params.back().first) {
            out << ", ";
        }
    }
    out << ") {\n";
    out << "}\n";
}

}