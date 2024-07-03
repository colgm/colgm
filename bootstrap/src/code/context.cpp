#include "colgm.h"
#include "code/context.h"
#include "pass/pass_manager.h"

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
}

void ir_context::dump_struct_size_method(std::ostream& out) const {
    for(const auto& st : struct_decls) {
        const auto st_type = type {
            .name = st.get_name(),
            .loc_file = st.get_location().file
        };
        const auto st_name = mangle_in_module_symbol(st_type.full_path_name());
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

void ir_context::dump_struct_alloc_method(std::ostream& out) const {
    for(const auto& st: struct_decls) {
        const auto st_type = type {
            .name = st.get_name(),
            .loc_file = st.get_location().file
        };
        const auto st_name = mangle_in_module_symbol(st_type.full_path_name());
        const auto st_real_name = "%struct." + st_name;
        out << "define " << st_real_name << "* @" << st.get_name();
        out << ".__alloc__() alwaysinline {\n";
        out << "entry:\n";
        out << "  %0 = call i64 @" << st_name << ".__size__()\n";
        out << "  %1 = call i8* @malloc(i64 %0)\n";
        out << "  %2 = bitcast i8* %1 to " << st_real_name << "*\n";
        out << "  ret " << st_real_name << "* %2\n";
        out << "}\n";
    }
}

void ir_context::dump_struct_delete_method(std::ostream& out) const {
    for(const auto& st: struct_decls) {
        const auto st_type = type {
            .name = st.get_name(),
            .loc_file = st.get_location().file
        };
        const auto st_name = mangle_in_module_symbol(st_type.full_path_name());
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

void ir_context::dump_code(std::ostream& out) {
    if (!passes_already_executed) {
        pass_manager().run(*this);
        passes_already_executed = true;
    }

    for(const auto& i : struct_decls) {
        i.dump(out);
    }
    dump_const_string(out);
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
}

}