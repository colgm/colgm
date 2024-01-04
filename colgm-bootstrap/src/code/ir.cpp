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

}