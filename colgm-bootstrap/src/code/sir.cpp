#include "code/sir.h"
#include "colgm.h"

namespace colgm {

void sir_alloca::dump(std::ostream& out) const {
    out << "%" << variable_name << " = alloca " << type_name << "\n";
}

sir_block::~sir_block() {
    for(auto i : allocs) {
        delete i;
    }
    for(auto i : stmts) {
        delete i;
    }
}

void sir_block::dump(std::ostream& out) const {
    for(auto i : allocs) {
        out << "  ";
        i->dump(out);
    }
    if (allocs.size() && stmts.size()) {
        out << "\n";
    }
    for(auto i : stmts) {
        if (i->get_ir_type()!=sir_kind::sir_label) {
            out << "  ";
        }
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

void sir_string::dump(std::ostream& out) const {
    out << "%" << destination << " = getelementptr ";
    out << "[" << literal.size()+1 << " x i8], ";
    out << "[" << literal.size()+1 << " x i8]* @.constant.str." << index;
    out << ", i64 0, i64 0\n";
}

void sir_call_index::dump(std::ostream& out) const {
    out << "%" << destination << " = %" << source;
    out << "[%" << index << "]\n";
}

void sir_call_field::dump(std::ostream& out) const {
    out << "; call_field " << field_name << "\n";
}

void sir_ptr_call_field::dump(std::ostream& out) const {
    out << "; ptr_callfld " << field_name << "\n";
}

void sir_call_func::dump(std::ostream& out) const {
    out << "call @" << name << "(";
    for(const auto& i : args) {
        out << i;
        if (i!=args.back()) {
            out << ", ";
        }
    }
    out << ")\n";
}

void sir_binary::dump(std::ostream& out) const {
    out << "; calc \"" << opr << "\"\n";
}

void sir_label::dump(std::ostream& out) const {
    out << ".label_" << label_count << ":\n";
}

void sir_assign::dump(std::ostream& out) const {
    out << "; assign " << opr << "\n";
}

void sir_store::dump(std::ostream& out) const {
    out << "store " << type << " %" << source;
    out << ", " << type << "* %" << destination << "\n";
}

void sir_br::dump(std::ostream& out) const {
    out << "br label %.label_" << destination << "\n";
}

void sir_br_cond::dump(std::ostream& out) const {
    out << "br_cond ";
    out << "label %.label_" << destination_true << ", ";
    out << "label %.label_" << destination_false << "\n";
}

}