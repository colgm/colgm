#include "code/sir.h"
#include "colgm.h"

namespace colgm {

void sir_alloca::dump(std::ostream& out) const {
    out << "%" << variable_name << " = alloca " << type_name << "\n";
}

sir_block::~sir_block() {
    for(auto i : stmts) {
        delete i;
    }
}

void sir_block::dump(std::ostream& out) const {
    for(auto i : stmts) {
        if (i->get_ir_type()!=sir_kind::sir_label) {
            out << "  ";
        }
        i->dump(out);
    }
}

void sir_number::dump(std::ostream& out) const {
    out << "%" << destination << " = ";
    out << (is_integer? "add":"fadd") << " " << type_name << " ";
    out << literal << ", ";
    out << (is_integer? "0":"0.0");
    out << "\n";
}

void sir_ret::dump(std::ostream& out) const {
    out << "ret " << type;
    if (value.size()) {
        out << " %" << value;
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
    out << "%" << destination << " = getelementptr " << type << ", ";
    out << type << "* %" << source << ", ";
    out << index_type << " %" << index << "\n";
}

void sir_call_field::dump(std::ostream& out) const {
    out << "%" << destination << " = getelementptr inbounds ";
    out << struct_name << ", ";
    out << struct_name << "* %" << source << ", i32 0, i32 " << index << "\n";
}

void sir_call_func::dump(std::ostream& out) const {
    if (destination.size()) {
        out << "%" << destination << " = ";
    }
    out << "call " << return_type << " @" << name << "(";
    for(usize i = 0; i<args.size(); ++i) {
        out << args_type[i] << " %" << args[i];
        if (i!=args.size()-1) {
            out << ", ";
        }
    }
    out << ")\n";
}

void sir_binary::dump(std::ostream& out) const {
    out << "%" << destination << " = " << opr << " " << type << " ";
    out << "%" << left << ", %" << right << "\n";
}

void sir_label::dump(std::ostream& out) const {
    out << ".label_" << label_count << ":\n";
}

void sir_store::dump(std::ostream& out) const {
    out << "store " << type << " %" << source;
    out << ", " << type << "* %" << destination << "\n";
}

void sir_store_literal::dump(std::ostream& out) const {
    out << "store " << type << " " << source;
    out << ", " << type << "* %" << destination << "\n";
}

void sir_load::dump(std::ostream& out) const {
    out << "%" << destination << " = load " << type;
    out << ", " << type << "* %" << source << "\n";
}

void sir_br::dump(std::ostream& out) const {
    out << "br label %.label_" << destination << "\n";
}

void sir_br_cond::dump(std::ostream& out) const {
    out << "br i1 %" << condition << ", ";
    out << "label %.label_" << destination_true << ", ";
    out << "label %.label_" << destination_false << "\n";
}

}