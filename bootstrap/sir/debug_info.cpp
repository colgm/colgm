#include "sir/debug_info.h"

namespace colgm {

void DI_node::dump(std::ostream& out) const {
    dump_index(out);
    out << "!undef";
}

void DI_named_metadata::dump(std::ostream& out) const {
    dump_index(out);
    out << "!" << name << " = !{";
    for (auto n : nodes) {
        n->dump(out);
        if (n != nodes.back()) {
            out << ", ";
        }
    }
    out << "}";
}

void DI_ref_index::dump(std::ostream& out) const {
    dump_index(out);
    out << "!" << ref_index;
}

void DI_list::dump(std::ostream& out) const {
    dump_index(out);
    out << "!{";
    for (auto n : nodes) {
        n->dump(out);
        if (n != nodes.back()) {
            out << ", ";
        }
    }
    out << "}";
}

void DI_i32::dump(std::ostream& out) const {
    dump_index(out);
    out << "i32 " << value;
}

void DI_string::dump(std::ostream& out) const {
    dump_index(out);
    out << "!\"" << value << "\"";
}

void DI_file::dump(std::ostream& out) const {
    dump_index(out);
    out << "!DIFile(filename: \"" << filename;
    out << "\", directory: \"" << directory << "\")";
}

}