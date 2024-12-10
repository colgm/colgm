#include "colgm.h"
#include "sema/symbol.h"
#include "sir/context.h"

#include <iomanip>

namespace colgm {

const auto sir_struct::get_mangled_name() const {
    const auto ty = type {
        .name = name,
        .loc_file = location.file
    };
    return quoted_name("struct." + mangle(ty.full_path_name()));
}

void sir_struct::dump(std::ostream& out) const {
    out << "%" << get_mangled_name();
    if (field_type.empty()) {
        out << " = type {}\n";
        return;
    }
    out << " = type { ";
    for(usize i = 0; i<field_type.size(); ++i) {
        out << quoted_name(field_type[i]);
        if (i != field_type.size()-1) {
            out << ", ";
        }
    }
    out << " }\n";
}

void sir_func::dump_attributes(std::ostream& out) const {
    bool contain_frame_pointer_attr = false;
    for(const auto& i : attributes) {
        out << " " << i;
        if (i.find("frame-pointer") != std::string::npos) {
            contain_frame_pointer_attr = true;
        }
    }
    // add frame pointer attribute for macos
    // then backtrace can get symbol of functions
    // but on linux, option `-rdynamic` should be given to ld
    // otherwise this attribute does not work
    if (block && !contain_frame_pointer_attr &&
        std::string(get_platform()) == "macos") {
        out << " \"frame-pointer\"=\"non-leaf\"";
    }
}

void sir_func::dump(std::ostream& out) const {
    out << (block? "define ":"declare ");
    out << quoted_name(return_type) << " @" << get_mangled_name() << "(";
    for(const auto& i : params) {
        out << quoted_name(i.second) << " %" << i.first;
        if (i.first != params.back().first) {
            out << ", ";
        }
    }
    out << ")";

    dump_attributes(out);

    if (debug_info_index != DI_node::DI_ERROR_INDEX) {
        out << " !dbg !" << debug_info_index;
    }
    if (!block) {
        out << "\n";
        return;
    }
    out << " {\n";
    out << "label.entry:\n";
    block->dump(out);
    out << "}\n";
}

void sir_context::dump_target_tripple(std::ostream& out) const {
    const auto platform = std::string(get_platform());
    const auto arch = std::string(get_arch());

    if (platform == "linux" && arch == "x86_64") {
        out << "target triple = \"x86_64-pc-linux-gnu\"\n";
    }
    if (platform == "macos" && arch == "aarch64") {
        out << "target triple = \"arm64-apple-macosx12.0.0\"\n";
    }
}

void sir_context::dump_const_string(std::ostream& out) const {
    // make sure constant strings are in order
    std::vector<std::string> ordered_const_string;
    ordered_const_string.resize(const_strings.size());
    for(const auto& i : const_strings) {
        ordered_const_string[i.second] = i.first;
    }

    for(usize i = 0; i<ordered_const_string.size(); ++i) {
        out << "@.str." << i;
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
        const auto st_real_name = quoted_name("%struct." + st_name);
        const auto size_func_name = quoted_name(st_name + ".__size__");
        out << "define i64 @" << size_func_name << "() alwaysinline {\n";
        out << "label.entry:\n";
        out << "  ret i64 ptrtoint (" << st_real_name;
        out << "* getelementptr (" << st_real_name << ", ";
        out << st_real_name << "* null, i64 1) to i64)\n";
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
        const auto st_real_name = quoted_name("%struct." + st_name);
        const auto alloc_func_name = quoted_name(st_name + ".__alloc__");
        const auto size_func_name = quoted_name(st_name + ".__size__");
        out << "define " << st_real_name << "* ";
        out << "@" << alloc_func_name;
        out << "() alwaysinline {\n";
        out << "label.entry:\n";
        out << "  %0 = call i64 @" << size_func_name << "()\n";
        out << "  %1 = call i8* @malloc(i64 %0)\n";
        out << "  %2 = bitcast i8* %1 to " << st_real_name << "*\n";
        out << "  ret " << st_real_name << "* %2\n";
        out << "}\n";
    }
}

void sir_context::dump_code(std::ostream& out) {
    // generate target triple
    dump_target_tripple(out);

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
    for (auto i : func_impls) {
        i->dump(out);
        out << "\n";
    }

    for (auto i : named_metadata) {
        i->dump(out);
        out << "\n";
    }

    if (named_metadata.size() && debug_info.size()) {
        out << "\n";
    }

    for (auto i : debug_info) {
        i->dump(out);
        out << "\n";
    }
}

}