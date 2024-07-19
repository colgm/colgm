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

}