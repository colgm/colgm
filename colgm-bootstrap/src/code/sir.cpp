#include "code/sir.h"
#include "colgm.h"

namespace colgm {

void sir_alloca::dump(std::ostream& out) const {
    out << variable_name << " = alloca " << type_name << "\n";
}

sir_code_block::~sir_code_block() {
    for(auto i : stmts) {
        delete i;
    }
}

void sir_code_block::dump(std::ostream& out) const {
    for(auto i : stmts) {
        out << "  ";
        i->dump(out);
    }
}

void sir_ret::dump(std::ostream& out) const {
    out << "ret";
    if (!has_return_value) {
        out << " void";
    }
    out << "\n";
}

void sir_number::dump(std::ostream& out) const {
    out << "push_number  " << literal << "\n";
}

void sir_string::dump(std::ostream& out) const {
    out << "push_string  \"" << rawstr(literal) << "\"\n";
}

void sir_bool::dump(std::ostream& out) const {
    out << "push_bool " << (flag? "true":"false") << "\n";
}

void sir_call_index::dump(std::ostream& out) const {
    out << "call_index\n";
}

void sir_call_field::dump(std::ostream& out) const {
    out << "call_field   " << field_name << "\n";
}

void sir_ptr_call_field::dump(std::ostream& out) const {
    out << "ptr_callfld  " << field_name << "\n";
}

void sir_call_path::dump(std::ostream& out) const {
    out << "call_path    " << field_name << "\n";
}

void sir_call_func::dump(std::ostream& out) const {
    out << "call_funct   " << argc << "\n";
}

void sir_get_var::dump(std::ostream& out) const {
    out << "get_variable " << name << "\n";
}

void sir_binary::dump(std::ostream& out) const {
    out << "calculation  " << opr << "\n";
}

void sir_label::dump(std::ostream& out) const {
    out << "label " << label_count << ":\n";
}

void sir_assign::dump(std::ostream& out) const {
    out << "store_value  " << opr << "\n";
}

void sir_br_direct::dump(std::ostream& out) const {
    out << "br_direct    label " << destination << "\n";
}

void sir_br_cond::dump(std::ostream& out) const {
    out << "br_cond      ";
    out << "label_true " << destination_true << ", ";
    out << "label_false " << destination_false << "\n";
}

}