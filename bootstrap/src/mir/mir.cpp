#include "mir/mir.h"

#include <iomanip>

namespace colgm::mir {

void mir_nop::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "] nop\n";
}

void mir_block::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "] block {";
    if (content.empty()) {
        os << "}\n";
        return;
    }

    os << "\n";
    for(auto i : content) {
        i->dump(indent + "  ", os);
    }
    os << indent << "}\n";
}

void mir_unary::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "] unary ";
    switch(opr) {
        case opr_kind::neg: os << "-"; break;
        case opr_kind::bnot: os << "~"; break;
    }
    os << " {\n";
    for(auto i : value->get_content()) {
        i->dump(indent + "  ", os);
    }
    os << indent << "}\n";
}

void mir_binary::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "] binary ";
    switch(opr) {
        case opr_kind::add: os << "+"; break;
        case opr_kind::sub: os << "-"; break;
        case opr_kind::mult: os << "*"; break;
        case opr_kind::div: os << "/"; break;
        case opr_kind::rem: os << "%"; break;
        case opr_kind::cmpeq: os << "=="; break;
        case opr_kind::cmpneq: os << "!="; break;
        case opr_kind::less: os << "<"; break;
        case opr_kind::leq: os << "<="; break;
        case opr_kind::grt: os << ">"; break;
        case opr_kind::geq: os << ">="; break;
        case opr_kind::cmpand: os << "&&"; break;
        case opr_kind::cmpor: os << "||"; break;
        case opr_kind::band: os << "&"; break;
        case opr_kind::bor: os << "|"; break;
        case opr_kind::bxor: os << "^"; break;
    }
    os << " {\n";
    for(auto i : left->get_content()) {
        i->dump(indent + "  ", os);
    }
    os << indent << "}.{\n";
    for(auto i : right->get_content()) {
        i->dump(indent + "  ", os);
    }
    os << indent << "}\n";
}

void mir_type_convert::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[target:" << target_type << "]";
    os << " type convert {\n";
    for(auto i : source->get_content()) {
        i->dump(indent + "  ", os);
    }
    os << indent << "}\n";
}

void mir_nil::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "]";
    os << " nil\n";
}

void mir_number::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "]";
    os << " number:" << literal << "\n";
}

void mir_string::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "]";
    os << " string:\"";
    for(const auto i : literal) {
        os << "\\";
        os << std::hex << std::setw(2) << std::setfill('0') << int(i) << std::dec;
    }
    os << "\"\n";
}

void mir_char::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "]";
    os << " char:\'";
    os << "\\";
    os << std::hex << std::setw(2) << std::setfill('0') << int(literal);
    os << std::dec << "\'\n";
}

void mir_bool::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "]";
    os << " bool:" << (literal? "true":"false") << "\n";
}

void mir_call::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "] call {\n";
    for(auto i : content->get_content()) {
        i->dump(indent + "  ", os);
    }
    os << indent << "}\n";
}

void mir_call_id::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "]";
    os << (resolve_type.is_global? "[global]":"[instance]");
    os << " identifier:" << name << "\n";
}

void mir_call_index::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "] call index {\n";
    for(auto i : index->get_content()) {
        i->dump(indent + "  ", os);
    }
    os << indent << "}\n";
}

void mir_call_func::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "][" << arg_size << "]";
    os << " call func\n";
}

void mir_call_field::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "]";
    os << " call field:" << name << "\n";
}

void mir_ptr_call_field::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "]";
    os << " pointer call field:" << name << "\n";
}

void mir_call_path::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "]";
    os << (resolve_type.is_global? "[global]":"[instance]");
    os << " call path:" << name << "\n";
}

void mir_define::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "]";
    os << "[" << resolve_type << "]";
    os << "[expect:" << expect_type << "]";
    os << " define:" << name << " {\n";
    for(auto i : init_value->get_content()) {
        i->dump(indent + "  ", os);
    }
    os << indent << "}\n";
}

void mir_assign::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "] assign ";
    switch(opr) {
       case opr_kind::eq: os << "="; break;
       case opr_kind::addeq: os << "+="; break;
       case opr_kind::subeq: os << "-="; break;
       case opr_kind::multeq: os << "*="; break;
       case opr_kind::diveq: os << "/="; break;
       case opr_kind::remeq: os << "%="; break;
       case opr_kind::andeq: os << "&="; break;
       case opr_kind::xoreq: os << "^="; break;
       case opr_kind::oreq: os << "|="; break;
    }
    os << " {\n";
    left->dump(indent + "  ", os);
    right->dump(indent + "  ", os);
    os << indent << "}\n";
}

void mir_if::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "] ";
    os << (condition? "if":"else");
    os << " {\n";
    if (condition) {
        condition->dump(indent + "  ", os);
    }
    content->dump(indent + "  ", os);
    os << indent << "}\n";
}

void mir_branch::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "] branch {\n";
    for(auto i : branch) {
        i->dump(indent + "  ", os);
    }
    os << indent << "}\n";
}

void mir_break::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "] break\n";
}

void mir_continue::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "] continue\n";
}

void mir_while::dump(const std::string& indent, std::ostream& os) {
    os << indent << "[" << location << "] while {\n";
    condition->dump(indent + "  ", os);
    content->dump(indent + "  ", os);
    os << indent << "}\n";
}

}