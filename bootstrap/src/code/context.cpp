#include "code/context.h"

#include <iomanip>

namespace colgm {

void ir_context::dump_raw_string(std::ostream& out,
                                 const std::string& src) const {
    for(const auto c : src) {
        if (c == '\\') {
            out << "\\\\";
        } else if (32 <= c && c <= 126) {
            out << c;
        } else {
            out << "\\";
            out << std::hex << std::setw(2) << std::setfill('0') << u32(c);
            out << std::dec;
        }
    }
    out << "\\00";
}

void ir_context::dump_const_string(std::ostream& out) const {
    for(const auto& i : const_strings) {
        out << "@.constant.str." << i.second;
        out << " = private unnamed_addr constant [";
        out << i.first.length() + 1 << " x i8] c\"";
        dump_raw_string(out, i.first);
        out << "\"\n";
    }
}

std::string ir_context::convert_instruction(char before_type_mark,
                                            int before_bit_size,
                                            char after_type_mark,
                                            int after_bit_size) const {
    if (before_type_mark=='u' && after_type_mark=='u') {
        return before_bit_size<after_bit_size? "zext":"trunc";
    }
    if (before_type_mark=='u' && after_type_mark=='i') {
        return before_bit_size<after_bit_size? "zext":"trunc";
    }
    if (before_type_mark=='u' && after_type_mark=='f') {
        return "uitofp";
    }
    if (before_type_mark=='i' && after_type_mark=='u') {
        return before_bit_size<after_bit_size? "sext":"trunc";
    }
    if (before_type_mark=='i' && after_type_mark=='i') {
        return before_bit_size<after_bit_size? "sext":"trunc";
    }
    if (before_type_mark=='i' && after_type_mark=='f') {
        return "sitofp";
    }
    if (before_type_mark=='f' && after_type_mark=='u') {
        return "fptoui";
    }
    if (before_type_mark=='f' && after_type_mark=='i') {
        return "fptosi";
    }
    if (before_type_mark=='f' && after_type_mark=='f') {
        return before_bit_size<after_bit_size? "fpext":"fptrunc";
    }
    // unreachable
    return "";
}

void ir_context::dump_used_basic_convert_method(std::ostream& out) const {
    // TODO: this is a demo, fix it later
    const std::unordered_map<std::string, std::string> type_mapper = {
        {"u8", "i8"}, {"u16", "i16"}, {"u32", "i32"}, {"u64", "i64"},
        {"i8", "i8"}, {"i16", "i16"}, {"i32", "i32"}, {"i64", "i64"},
        {"f32", "float"}, {"f64", "double"}
    };
    for(const auto& i : used_basic_convert_method) {
        const auto before_type_mark = i.first[0];
        const auto before_bit_size = std::stoi(i.first.substr(1));
        for(const auto& convert_type : i.second) {
            const auto after_type_mark = convert_type[0];
            const auto after_bit_size = std::stoi(convert_type.substr(1));
            out << "define " << type_mapper.at(convert_type) << " ";
            out << "@native." << i.first << "." << convert_type;
            out << "(" << type_mapper.at(i.first) << " %num) alwaysinline {\n";
            if (type_mapper.at(i.first)==type_mapper.at(convert_type)) {
                out << "  ret " << type_mapper.at(convert_type) << " %num\n";
            } else {
                out << "  %1 = ";
                out << convert_instruction(
                    before_type_mark,
                    before_bit_size,
                    after_type_mark,
                    after_bit_size
                );
                out << " " << type_mapper.at(i.first);
                out << " %num to " << type_mapper.at(convert_type) << "\n";
                out << "  ret " << type_mapper.at(convert_type) << " %1\n";
            }
            out << "}\n";
        }
    }
}

void ir_context::dump_struct_size_method(std::ostream& out) const {
    for(const auto& st : struct_decls) {
        out << "define i64 @" << st.get_name() << ".__size__() alwaysinline {\n";
        out << "  %1 = getelementptr %struct." << st.get_name();
        out << ", %struct." << st.get_name() << "* null, i64 1\n";
        out << "  %2 = ptrtoint %struct." << st.get_name() << "* %1 to i64\n";
        out << "  ret i64 %2\n";
        out << "}\n";
    }
}

void ir_context::dump_struct_alloc_method(std::ostream& out) const {
    for(const auto& st: struct_decls) {
        out << "define %struct." << st.get_name() << "* @" << st.get_name();
        out << ".__alloc__() alwaysinline {\n";
        out << "  %1 = call i64 @" << st.get_name() << ".__size__()\n";
        out << "  %2 = call i8* @malloc(i64 %1)\n";
        out << "  %3 = bitcast i8* %2 to %struct." << st.get_name() << "*\n";
        out << "  ret %struct." << st.get_name() << "* %3\n";
        out << "}\n";
    }
}

void ir_context::dump_struct_delete_method(std::ostream& out) const {
    for(const auto& st: struct_decls) {
        out << "define void @" << st.get_name();
        out << ".__delete__(%struct." << st.get_name();
        out << "* %self) alwaysinline {\n";
        out << "  %1 = bitcast %struct." << st.get_name() << "* %self to i8*\n";
        out << "  call void @free(i8* %1)\n";
        out << "  ret void\n";
        out << "}\n";
    }
}

void ir_context::check_and_dump_default_main(std::ostream& out) const {
    for(auto i : func_decls) {
        if (i->get_name()=="@main") {
            return;
        }
    }
    for(auto i : func_impls) {
        if (i->get_name()=="@main") {
            return;
        }
    }
    out << "define i32 @main(i32 %argc, i8** %argv) {\n";
    out << "  ret i32 0;\n";
    out << "}\n";
}

void ir_context::dump_code(std::ostream& out) const {
    for(const auto& i : struct_decls) {
        i.dump(out);
    }
    dump_const_string(out);
    dump_used_basic_convert_method(out);
    dump_struct_size_method(out);
    dump_struct_alloc_method(out);
    dump_struct_delete_method(out);
    if (struct_decls.size()) {
        out << "\n";
    }
    for(auto i : func_decls) {
        i->dump(out);
    }
    if (func_decls.size()) {
        out << "\n";
    }
    for(auto i : func_impls) {
        i->dump(out);
        out << "\n";
    }
    check_and_dump_default_main(out);
}

}