#include "code/context.h"

#include <iomanip>

namespace colgm {

void ir_context::dump_raw_string(std::ostream& out,
                                 const std::string& src) const {
    for(const auto c : src) {
        if (32 <= c && c <= 126) {
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

void ir_context::dump_struct_size_method(std::ostream& out) const {
    for(const auto& st : struct_decls) {
        out << "define i64 @" << st.get_name() << ".__size__() {\n";
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
        out << ".__alloc__() {\n";
        out << "  %1 = call i64 @" << st.get_name() << ".__size__()\n";
        out << "  %2 = call i8* @malloc(i64 %1)\n";
        out << "  %3 = bitcast i8* %2 to %struct." << st.get_name() << "*\n";
        out << "  ret %struct." << st.get_name() << "* %3\n";
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
    dump_struct_size_method(out);
    dump_struct_alloc_method(out);
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