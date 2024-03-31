#include "code/hir.h"

namespace colgm {

void hir_struct::dump(std::ostream& out) const {
    out << "%struct." << name << " = type {";
    for(usize i = 0; i<field_type.size(); ++i) {
        out << field_type[i];
        if (i!=field_type.size()-1) {
            out << ", ";
        }
    }
    out << "}\n";
}

hir_func::~hir_func() {
    delete cb;
}

void hir_func::dump(std::ostream& out) const {
    out << (cb? "define ":"declare ");
    out << return_type << " " << name << "(";
    for(const auto& i : params) {
        out << i.second << " %" << i.first;
        if (i.first!=params.back().first) {
            out << ", ";
        }
    }
    out << ")";
    if (!cb) {
        out << "\n";
        return;
    }
    out << " {\n";
    out << "entry:\n";
    cb->dump(out);
    out << "}\n";
}

}