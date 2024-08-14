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
    out << "%struct." << mangle(ty.full_path_name());
    out << " = type {";
    for(usize i = 0; i<field_type.size(); ++i) {
        out << field_type[i];
        if (i!=field_type.size()-1) {
            out << ", ";
        }
    }
    out << "}\n";
}

void sir_func::dump(std::ostream& out) const {
    out << (block? "define ":"declare ");
    out << return_type << " " << name << "(";
    for(const auto& i : params) {
        out << i.second << " %" << i.first;
        if (i.first!=params.back().first) {
            out << ", ";
        }
    }
    out << ")";
    if (!block) {
        out << "\n";
        return;
    }
    out << " {\n";
    out << "entry:\n";
    block->dump(out);
    out << "}\n";
}

void sir_context::dump_raw_string(std::ostream& out,
                                  const std::string& src) const {
    for(const auto c : src) {
        if (std::isdigit(c) || std::isalpha(c)) {
            out << c;
            continue;
        }
        out << "\\";
        out << std::hex << std::setw(2) << std::setfill('0') << u32(c);
        out << std::dec;
    }
    out << "\\00";
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
        dump_raw_string(out, ordered_const_string[i]);
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
            .loc_file = st->get_location().file
        };
        const auto st_name = mangle(st_type.full_path_name());
        const auto st_real_name = "%struct." + st_name;
        out << "define i64 @" << st_name;
        out << ".__size__() alwaysinline {\n";
        out << "entry:\n";
        out << "  %0 = getelementptr " << st_real_name;
        out << ", " << st_real_name << "* null, i64 1\n";
        out << "  %1 = ptrtoint " << st_real_name << "* %0 to i64\n";
        out << "  ret i64 %1\n";
        out << "}\n";
    }
}

void sir_context::dump_struct_alloc_method(std::ostream& out) const {
    for(const auto& st: struct_decls) {
        const auto st_type = type {
            .name = st->get_name(),
            .loc_file = st->get_location().file
        };
        const auto st_name = mangle(st_type.full_path_name());
        const auto st_real_name = "%struct." + st_name;
        out << "define " << st_real_name << "* @" << st_name;
        out << ".__alloc__() alwaysinline {\n";
        out << "entry:\n";
        out << "  %0 = call i64 @" << st_name << ".__size__()\n";
        out << "  %1 = call i8* @malloc(i64 %0)\n";
        out << "  %2 = bitcast i8* %1 to " << st_real_name << "*\n";
        out << "  ret " << st_real_name << "* %2\n";
        out << "}\n";
    }
}

void sir_context::dump_struct_delete_method(std::ostream& out) const {
    for(const auto& st : struct_decls) {
        const auto st_type = type {
            .name = st->get_name(),
            .loc_file = st->get_location().file
        };
        const auto st_name = mangle(st_type.full_path_name());
        const auto st_real_name = "%struct." + st_name;
        out << "define void @" << st_name;
        out << ".__delete__(" << st_real_name;
        out << "* %self) alwaysinline {\n";
        out << "entry:\n";
        out << "  %0 = bitcast " << st_real_name << "* %self to i8*\n";
        out << "  call void @free(i8* %0)\n";
        out << "  ret void\n";
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
    dump_struct_delete_method(out);
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
    out << "!0 = !{!\"colgm compiler version " << __colgm_ver__ << "\"}";
}

}