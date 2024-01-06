#include "code/ir.h"
#include "colgm.h"

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

ir_code_block::~ir_code_block() {
    for(auto i : stmts) {
        delete i;
    }
}

void ir_code_block::dump(std::ostream& out) {
    for(auto i : stmts) {
        out << "  ";
        i->dump(out);
    }
}

void ir_ret::dump(std::ostream& out) {
    out << "ret";
    if (!has_return_value) {
        out << " void";
    }
    out << "\n";
}

ir_func_decl::~ir_func_decl() {
    delete cb;
}

void ir_func_decl::dump(std::ostream& out) {
    if (cb) {
        out << "define ";
    } else {
        out << "declare ";
    }
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
    cb->dump(out);
    out << "}\n";
}

void ir_number::dump(std::ostream& out) {
    out << "push_number  " << literal << "\n";
}

void ir_string::dump(std::ostream& out) {
    out << "push_string  \"" << rawstr(literal) << "\"\n";
}

void ir_get_var::dump(std::ostream& out) {
    out << "get_variable " << name << "\n";
}

void ir_binary::dump(std::ostream& out) {
    out << "calculation  " << opr << "\n";
}

}