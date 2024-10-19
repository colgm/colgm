#include "colgm.h"
#include "sema/symbol.h"
#include "sir/context.h"

#include <iomanip>

namespace colgm {

void sir_struct::dump(std::ostream& out) const {
    const auto ty = type {
        .name = name,
        .loc_file = location.file
    };
    out << "%" << quoted_name("struct." + mangle(ty.full_path_name()));
    if (field_type.empty()) {
        out << " = type {}\n";
        return;
    }
    out << " = type { ";
    for(usize i = 0; i<field_type.size(); ++i) {
        out << field_type[i];
        if (i!=field_type.size()-1) {
            out << ", ";
        }
    }
    out << " }\n";
}

void sir_func::dump(std::ostream& out) const {
    out << (block? "define ":"declare ");
    out << quoted_name(return_type) << " " << quoted_name(name) << "(";
    for(const auto& i : params) {
        out << quoted_name(i.second) << " %" << i.first;
        if (i.first != params.back().first) {
            out << ", ";
        }
    }
    out << ")";
    if (!block) {
        out << "\n";
        return;
    }
    out << " {\n";
    out << "label.entry:\n";
    block->dump(out);
    out << "}\n";
}

void sir_context::dump_const_string(std::ostream& out) const {
    // make sure constant strings are in order
    std::vector<std::string> ordered_const_string;
    ordered_const_string.resize(const_strings.size());
    for(const auto& i : const_strings) {
        ordered_const_string[i.second] = i.first;
    }

    for(usize i = 0; i<ordered_const_string.size(); ++i) {
        out << "@const.str." << i;
        out << " = private unnamed_addr constant [";
        out << ordered_const_string[i].length() + 1 << " x i8] c\"";
        out << llvm_raw_string(ordered_const_string[i]);
        out << "\"\n";
    }

    if (ordered_const_string.size()) {
        out << "\n";
    }
}

void sir_context::dump_struct_size_method(std::ostream& out) const {
    for(const auto& st : struct_decls) {
        const auto st_type = type {
            .name = st->get_name(),
            .loc_file = st->get_file()
        };
        const auto st_name = mangle(st_type.full_path_name());
        const auto st_real_name = "%struct." + st_name;
        const auto size_func_name = quoted_name(st_name + ".__size__");
        out << "define i64 @" << size_func_name;
        out << "() alwaysinline {\n";
        out << "label.entry:\n";
        out << "  %0 = getelementptr " << quoted_name(st_real_name);
        out << ", " << quoted_name(st_real_name) << "* null, i64 1\n";
        out << "  %1 = ptrtoint " << quoted_name(st_real_name) << "* ";
        out << "%0 to i64\n";
        out << "  ret i64 %1\n";
        out << "}\n";
    }
}

void sir_context::dump_struct_alloc_method(std::ostream& out) const {
    for(const auto& st: struct_decls) {
        const auto st_type = type {
            .name = st->get_name(),
            .loc_file = st->get_file()
        };
        const auto st_name = mangle(st_type.full_path_name());
        const auto st_real_name = "%struct." + st_name;
        const auto alloc_func_name = quoted_name(st_name + ".__alloc__");
        const auto size_func_name = quoted_name(st_name + ".__size__");
        out << "define " << quoted_name(st_real_name) << "* ";
        out << "@" << alloc_func_name;
        out << "() alwaysinline {\n";
        out << "label.entry:\n";
        out << "  %0 = call i64 @" << size_func_name << "()\n";
        out << "  %1 = call i8* @malloc(i64 %0)\n";
        out << "  %2 = bitcast i8* %1 to " << quoted_name(st_real_name) << "*\n";
        out << "  ret " << quoted_name(st_real_name) << "* %2\n";
        out << "}\n";
    }
}

void sir_context::dump_code(std::ostream& out) {
    // generate declarations of structs
    for(auto i : struct_decls) {
        i->dump(out);
    }
    if (struct_decls.size()) {
        out << "\n";
    }

    // generate constant strings
    dump_const_string(out);

    // generate builtin methods for structs
    dump_struct_size_method(out);
    dump_struct_alloc_method(out);
    if (struct_decls.size()) {
        out << "\n";
    }

    // generate function declarations for normal functions or libc functions
    for(auto i : func_decls) {
        i->dump(out);
    }
    if (func_decls.size()) {
        out << "\n";
    }

    // generate implementations of functions
    for(auto i : func_impls) {
        i->dump(out);
        out << "\n";
    }

    out << "!llvm.ident = !{!0}\n";
    out << "!0 = !{!\"colgm compiler version " << __colgm_ver__ << "\"}\n";
}

}