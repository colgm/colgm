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

void DI_compile_unit::dump(std::ostream& out) const {
    dump_index(out);
    out << "distinct !DICompileUnit(";
    out << "language: DW_LANG_C99, ";
    out << "file: !" << file_index << ", ";
    out << "producer: \"" << producer << "\", ";
    out << "isOptimized: false)";
}

void DI_basic_type::dump(std::ostream& out) const {
    dump_index(out);
    out << "!DIBasicType(name: \"" << name << "\", ";
    out << "size: " << size_in_bits;
    out << ", encoding: " << encoding << ")";
}

void DI_structure_type::dump(std::ostream& out) const {
    dump_index(out);
    out << "!DICompositeType(tag: DW_TAG_structure_type, ";
    out << "name: \"" << name << "\", ";
    out << "file: !" << file_index << ", ";
    out << "line: " << line << ", ";
    out << "identifier: \"" << identifier << "\")";
}

void DI_enum_type::dump(std::ostream& out) const {
    dump_index(out);
    out << "!DICompositeType(tag: DW_TAG_enumeration_type, ";
    out << "name: \"" << name << "\", ";
    out << "file: !" << file_index << ", ";
    out << "line: " << line << ", ";
    out << "baseType: !" << base_type_index << ", ";
    out << "size: 64, flags: DIFlagEnumClass, ";
    if (elements_index != DI_node::DI_ERROR_INDEX) {
        out << "elements: !" << elements_index << ", ";
    }
    out << "identifier: \"" << identifier << "\")";
}

void DI_enumerator::dump(std::ostream& out) const {
    dump_index(out);
    out << "!DIEnumerator(name: \"" << name << "\", ";
    out << "value: " << value << ")";
}

}