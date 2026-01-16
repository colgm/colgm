#include "sir/sir.h"
#include "colgm.h"

#include <sstream>
#include <cstring>
#include <unordered_map>

namespace colgm {

std::string quoted_name(const std::string& str) {
    // make str copy
    auto copy = str;

    // find tail * sequence's beginning
    auto pos = str.size();
    while (pos > 0 && str[pos - 1] == '*') {
        pos--;
    }
    if (pos && pos != std::string::npos) {
        copy = str.substr(0, pos);
    }

    // if find any special characters, quote the name
    if (str.find_first_of("<:>") != std::string::npos) {
        copy = "\"" + copy + "\"";
    }

    // add tail * sequence back
    if (pos != std::string::npos) {
        copy += str.substr(pos);
    }

    // move % and @ to the beginning
    if (copy.length() >= 2 &&
        (copy.substr(0, 2) == "\"%" || copy.substr(0, 2) == "\"@")) {
        auto c = copy[1];
        copy = copy.substr(2);
        copy = "\"" + copy;
        copy = c + copy;
    }
    return copy;
}

void sir_alloca::dump(std::ostream& out) const {
    out << "%" << variable << " = alloca ";
    if (array_info.array_base_type.length()) {
        out << "[" << array_info.size << " x ";
        out << quoted_name(array_info.array_base_type) << "]";
    } else {
        out << quoted_name(type);
    }
    out << "\n";
}

sir_block::~sir_block() {
    for (auto i : basic_blocks) {
        delete i;
    }
}

void sir_block::dump(std::ostream& out) const {
    for (auto i : basic_blocks) {
        if (i != basic_blocks.front()) {
            out << "\n";
        }
        i->dump(out);
    }
}

void sir_temp_ptr::dump(std::ostream& out) const {
    out << "%" << target << " = ";
    out << "getelementptr " << quoted_name(type) << ", ";
    out << quoted_name(type) << "* %" << source << ", i32 0";
    if (comment.empty()) {
        out << " ; %" << source << " -> %" << target << "\n";
    } else {
        out << " ; " << comment << "\n";
    }
}

void sir_ret::dump(std::ostream& out) const {
    out << "ret " << quoted_name(type) << " " << value << "\n";
}

void sir_string::dump(std::ostream& out) const {
    out << target << " = bitcast ";
    out << "[" << length << " x i8]* @str." << index;
    out << " to i8*\n";
}

void sir_zeroinitializer::dump(std::ostream& out) const {
    out << "store " << quoted_name(type) << " zeroinitializer";
    out << ", ptr " << target << "\n";
}

void sir_get_index::dump(std::ostream& out) const {
    out << destination << " = getelementptr " << quoted_name(type) << ", ";
    out << "ptr " << source << ", ";
    out << quoted_name(index_type) << " " << index << "\n";
}

void sir_get_field::dump(std::ostream& out) const {
    out << destination << " = getelementptr inbounds ";
    out << quoted_name(struct_name) << ", ";
    out << "ptr " << source << ", ";
    out << "i32 0, i32 " << index << "\n";
}

void sir_call::dump(std::ostream& out) const {
    if (destination.value_kind == value_t::kind::variable) {
        out << destination << " = ";
    }
    out << "call " << quoted_name(return_type);

    if (with_va_args) {
        out << "(";
        for (usize i = 0; i < with_va_args_real_param_size; ++i) {
            out << quoted_name(args_type[i]) << ", ";
        }
        out << "...)";
    }

    out << " @" << quoted_name(name) << "(";
    for (usize i = 0; i<args.size(); ++i) {
        out << quoted_name(args_type[i]) << " " << args[i];
        if (i != args.size()-1) {
            out << ", ";
        }
    }
    out << ")";
    if (debug_info_index != DI_node::DI_ERROR_INDEX) {
        out << ", !dbg !" << debug_info_index;
    }
    out << "\n";
}

void sir_neg::dump(std::ostream& out) const {
    out << destination << " = ";
    out << (is_integer? "sub":"fsub");
    out << " " << quoted_name(type) << " ";
    out << (is_integer? "0":"0.0");
    out << ", " << source << "\n";
}

void sir_bnot::dump(std::ostream& out) const {
    out << destination << " = xor " << quoted_name(type);
    out << " " << source << ", -1\n";
}

void sir_lnot::dump(std::ostream& out) const {
    out << destination << " = xor " << quoted_name(type);
    out << " " << source << ", true\n";
}

void sir_add::dump(std::ostream& out) const {
    out << destination << " = add ";
    out << quoted_name(type) << " " << left << ", " << right << "\n";
}

void sir_fadd::dump(std::ostream& out) const {
    out << destination << " = fadd ";
    out << quoted_name(type) << " " << left << ", " << right << "\n";
}

void sir_sub::dump(std::ostream& out) const {
    out << destination << " = ";
    out << (is_integer? "sub":"fsub") << " ";
    out << quoted_name(type) << " " << left << ", " << right << "\n";
}

void sir_mul::dump(std::ostream& out) const {
    out << destination << " = ";
    out << (is_integer? "mul":"fmul") << " ";
    out << quoted_name(type) << " " << left << ", " << right << "\n";
}

void sir_div::dump(std::ostream& out) const {
    out << destination << " = ";
    if (is_integer) {
        out << (is_signed? "sdiv":"udiv") << " ";
    } else {
        out << "fdiv ";
    }
    out << quoted_name(type) << " " << left << ", " << right << "\n";
}

void sir_rem::dump(std::ostream& out) const {
    out << destination << " = ";
    if (is_integer) {
        out << (is_signed? "srem":"urem") << " ";
    } else {
        out << "frem ";
    }
    out << quoted_name(type) << " " << left << ", " << right << "\n";
}

void sir_band::dump(std::ostream& out) const {
    out << destination << " = and ";
    out << quoted_name(type) << " " << left << ", " << right << "\n";
}

void sir_bxor::dump(std::ostream& out) const {
    out << destination << " = xor ";
    out << quoted_name(type) << " " << left << ", " << right << "\n";
}

void sir_bor::dump(std::ostream& out) const {
    out << destination << " = or ";
    out << quoted_name(type) << " " << left << ", " << right << "\n";
}

void sir_cmp::dump(std::ostream& out) const {
    out << destination << " = " << (is_integer? "icmp":"fcmp") << " ";
    auto head = (is_integer? (is_signed? 's':'u'):'u');
    switch(cmp_type) {
        case kind::cmp_eq: out << (is_integer? "eq":"ueq"); break;
        case kind::cmp_neq: out << (is_integer? "ne":"une"); break;
        case kind::cmp_ge: out << head << "ge"; break;
        case kind::cmp_gt: out << head << "gt"; break;
        case kind::cmp_le: out << head << "le"; break;
        case kind::cmp_lt: out << head << "lt"; break;
    }
    out << " " << quoted_name(type) << " ";
    out << left << ", " << right << "\n";
}

sir_basic_block::~sir_basic_block() {
    for (auto i : stmts) {
        delete i;
    }
}

std::string sir_basic_block::get_label() const {
    std::stringstream ss;
    ss << "label.L" << std::hex << label_count << std::dec;
    return ss.str();
}

void sir_basic_block::dump(std::ostream& out) const {
    out << get_label() << ":";
    if (comment.length()) {
        out << "\t\t\t\t; " << comment;
    }
    if (!preds.empty() || !succs.empty()) {
        out << "\t;";
        for (auto i : preds) {
            out << " pred:" << i->get_label();
        }
        for (auto i : succs) {
            out << " succ:" << i->get_label();
        }
    }
    out << "\n";
    for (auto i : stmts) {
        out << "  ";
        i->dump(out);
    }
}

void sir_store::dump(std::ostream& out) const {
    out << "store " << quoted_name(type) << " " << source;
    out << ", ptr " << destination << "\n";
}

void sir_load::dump(std::ostream& out) const {
    out << destination << " = load " << quoted_name(type);
    out << ", ptr " << source << "\n";
}

void sir_br::dump(std::ostream& out) const {
    out << "br label %" << get_label() << "\n";
}

std::string sir_br::get_label() const {
    std::stringstream ss;
    ss << "label.L" << std::hex << label << std::dec;
    return ss.str();
}

void sir_br_cond::dump(std::ostream& out) const {
    out << "br i1 " << condition << ", ";
    out << "label %" << get_label_true() << ", ";
    out << "label %" << get_label_false() << "\n";
}

std::string sir_br_cond::get_label_true() const {
    std::stringstream ss;
    ss << "label.L" << std::hex << label_true << std::dec;
    return ss.str();
}

std::string sir_br_cond::get_label_false() const {
    std::stringstream ss;
    ss << "label.L" << std::hex << label_false << std::dec;
    return ss.str();
}

std::string sir_switch::get_default_label() const {
    std::stringstream ss;
    ss << "label.L" << std::hex << label_default << std::dec;
    return ss.str();
}

std::vector<std::string> sir_switch::get_case_labels() const {
    std::vector<std::string> labels;
    for (const auto& c : label_cases) {
        std::stringstream ss;
        ss << "label.L" << std::hex << c.second << std::dec;
        labels.push_back(ss.str());
    }
    return labels;
}

void sir_switch::dump(std::ostream& out) const {
    out << "switch i64 " << source << ", ";
    out << "label %" << get_default_label() << " [\n";
    for (const auto& c : label_cases) {
        out << "    i64 " << c.first << ", ";
        out << "label %label.L" << std::hex << c.second << std::dec << "\n";
    }
    out << "  ]\n";
}

std::string sir_type_convert::convert_instruction(char source_type_mark,
                                                  int source_bit_size,
                                                  char dest_type_mark,
                                                  int dest_bit_size) const {
    if (source_type_mark == 'u' && dest_type_mark == 'u') {
        return source_bit_size<dest_bit_size? "zext":"trunc";
    }
    if (source_type_mark == 'u' && dest_type_mark == 'i') {
        return source_bit_size<dest_bit_size? "zext":"trunc";
    }
    if (source_type_mark == 'u' && dest_type_mark == 'f') {
        return "uitofp";
    }
    if (source_type_mark == 'i' && dest_type_mark == 'u') {
        return source_bit_size<dest_bit_size? "sext":"trunc";
    }
    if (source_type_mark == 'i' && dest_type_mark == 'i') {
        return source_bit_size<dest_bit_size? "sext":"trunc";
    }
    if (source_type_mark == 'i' && dest_type_mark == 'f') {
        return "sitofp";
    }
    if (source_type_mark == 'f' && dest_type_mark == 'u') {
        return "fptoui";
    }
    if (source_type_mark == 'f' && dest_type_mark == 'i') {
        return "fptosi";
    }
    if (source_type_mark == 'f' && dest_type_mark == 'f') {
        return source_bit_size < dest_bit_size ? "fpext" : "fptrunc";
    }
    // unreachable
    return "";
}

void sir_type_convert::dump(std::ostream& out) const {
    if (src_type.back() == '*' && dst_type.back() == '*') {
        out << destination << " = bitcast " << quoted_name(src_type) << " ";
        out << source << " to ";
        out << quoted_name(dst_type) << "\n";
        return;
    }
    if (src_type.back() != '*' && dst_type.back() == '*') {
        out << destination << " = inttoptr ";
        out << quoted_name(src_type) << " " << source << " to ";
        out << quoted_name(dst_type) << "\n";
        return;
    }
    if (src_type.back() == '*' && dst_type.back() != '*') {
        out << destination << " = ptrtoint ";
        out << quoted_name(src_type) << " " << source << " to ";
        out << quoted_name(dst_type) << "\n";
        return;
    }

    // error, should be unreachable
    if ((src_type.front() == '%' && src_type.back() != '*') ||
        (dst_type.front() == '%' && dst_type.back() != '*')) {
        out << destination << " = unknown ";
        out << quoted_name(src_type) << " " << source << " to ";
        out << quoted_name(dst_type) << "\n";
        return;
    }

    // basic type will fall through here
    // be aware, if type is an integer, 'u8 u16 u32 u64' will be 'i8 i16 i32 i64' here
    const std::unordered_map<std::string, std::string> ft = {
        {"float", "f32"}, {"double", "f64"}
    };

    auto real_src_type = ft.count(src_type)? ft.at(src_type):src_type;
    auto real_dst_type = ft.count(dst_type)? ft.at(dst_type):dst_type;
    if (real_src_type[0] == 'i' && src_unsigned) {
        real_src_type[0] = 'u';
    }
    if (real_dst_type[0] == 'i' && dst_unsigned) {
        real_dst_type[0] = 'u';
    }

    const auto source_type_mark = real_src_type[0];
    const auto source_bit_size = std::stoi(real_src_type.substr(1));
    const auto dest_type_mark = real_dst_type[0];
    const auto dest_bit_size = std::stoi(real_dst_type.substr(1));

    // source type maybe the same as target type
    // because like i64 & u64 share the same llvm type `i64`
    // bitcast will not check if integer is signed
    if (src_type == dst_type &&
        (real_src_type[0] == 'i' || real_src_type[0] == 'u')) {
        out << destination << " = bitcast ";
        out << src_type << " " << source << " to " << dst_type;
        out << " ; " << real_src_type << " -> " << real_dst_type << "\n";
        return;
    }
    // f32 f64 also
    if (src_type == dst_type && real_src_type[0] == 'f') {
        out << destination << " = bitcast ";
        out << src_type << " " << source << " to " << dst_type;
        out << " ; " << real_src_type << " -> " << real_dst_type << "\n";
        return;
    }

    // get convert instruction
    const auto convert_inst = convert_instruction(
        source_type_mark,
        source_bit_size,
        dest_type_mark,
        dest_bit_size
    );

    out << destination << " = " << convert_inst << " ";
    out << src_type << " " << source << " to " << dst_type;
    out << " ; " << src_type << " -> " << dst_type << "\n";
}

void sir_array_cast::dump(std::ostream& out) const {
    out << destination << " = bitcast [" << array_size << " x ";
    out << quoted_name(type) << "]* " << source << " to ";
    out << quoted_name(type) << "*\n";
}

}