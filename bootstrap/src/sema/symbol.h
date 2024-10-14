#pragma once

#include "colgm.h"
#include "report.h"

#include <ostream>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace colgm {

enum class sym_kind {
    error_kind,
    basic_kind,
    enum_kind,
    struct_kind,
    func_kind,
    package_kind,
    module_kind
};

struct struct_method_info {
    bool flag_is_static = false; // mark static method
    bool flag_is_normal = false; // mark normal method
    std::string method_name;
};

struct type {
    std::string name;
    std::string loc_file;
    i64 pointer_depth = 0;
    bool is_global = false;
    bool is_global_func = false;
    bool is_enum = false;
    bool is_immutable_array_address = false;
    bool is_constant_type = false;
    bool is_generic_placeholder = false;

    struct_method_info stm_info;
    std::vector<type> generics = {};

private:
    void check_pointer_depth() const;

public:
    std::string to_string() const;
    std::string full_path_name() const;
    std::string generic_name() const;
    // return name for symbol table search
    // normal type: .name
    // generic type: .generic_name()
    std::string name_for_search() const {
        return generics.size() ? generic_name() : name;
    }
    bool operator==(const type& another) const {
        return name==another.name &&
               pointer_depth==another.pointer_depth;
    }
    bool operator!=(const type& another) const {
        return name!=another.name ||
               pointer_depth!=another.pointer_depth;
    }
    bool eq_no_ptr(const type& another) const {
        return generic_name() == another.generic_name();
    }
    friend std::ostream& operator<<(std::ostream&, const type&);

public:
    static const type error_type() { return {"<err>", "", 0}; }
    static const type u64_type(i64 ptrlvl = 0) { return {"u64", "", ptrlvl}; }
    static const type u32_type(i64 ptrlvl = 0) { return {"u32", "", ptrlvl}; }
    static const type u16_type(i64 ptrlvl = 0) { return {"u16", "", ptrlvl}; }
    static const type u8_type(i64 ptrlvl = 0) { return {"u8", "", ptrlvl}; }
    static const type i64_type(i64 ptrlvl = 0) { return {"i64", "", ptrlvl}; }
    static const type i32_type(i64 ptrlvl = 0) { return {"i32", "", ptrlvl}; }
    static const type i16_type(i64 ptrlvl = 0) { return {"i16", "", ptrlvl}; }
    static const type i8_type(i64 ptrlvl = 0) { return {"i8", "", ptrlvl}; }
    static const type f64_type(i64 ptrlvl = 0) { return {"f64", "", ptrlvl}; }
    static const type f32_type(i64 ptrlvl = 0) { return {"f32", "", ptrlvl}; }
    static const type void_type(i64 ptrlvl = 0) { return {"void", "", ptrlvl}; }
    static const type bool_type(i64 ptrlvl = 0) { return {"bool", "", ptrlvl}; }

public:
    bool is_error() const { return name=="<err>"; }
    bool is_unsigned() const {
        const auto& t = *this;
        return t==type::u8_type(t.pointer_depth) ||
               t==type::u16_type(t.pointer_depth) ||
               t==type::u32_type(t.pointer_depth) ||
               t==type::u64_type(t.pointer_depth) ||
               t.pointer_depth > 0;
    }
    bool is_integer() const {
        const auto& t = *this;
        return t==type::i8_type(t.pointer_depth) ||
               t==type::u8_type(t.pointer_depth) ||
               t==type::i16_type(t.pointer_depth) ||
               t==type::u16_type(t.pointer_depth) ||
               t==type::i32_type(t.pointer_depth) ||
               t==type::u32_type(t.pointer_depth) ||
               t==type::i64_type(t.pointer_depth) ||
               t==type::u64_type(t.pointer_depth) ||
               (this->is_enum && !t.pointer_depth);
    }
    bool is_float() const {
        const auto& t = *this;
        return t==type::f32_type(t.pointer_depth) ||
               t==type::f64_type(t.pointer_depth);
    }
    bool is_boolean() const {
        return *this==type::bool_type(this->pointer_depth);
    }
    bool is_void() const { return *this==type::void_type(0); }
    bool is_pointer() const { return pointer_depth>0; }
    bool is_function() const {
        return stm_info.flag_is_normal ||
               stm_info.flag_is_static ||
               is_global_func;
    }

public:
    auto get_pointer_copy() const {
        auto copy = *this;
        ++copy.pointer_depth;
        return copy;
    }
    auto get_ref_copy() const {
        auto copy = *this;
        --copy.pointer_depth;
        return copy;
    }
};

struct symbol {
    std::string name; // symbol name
    type symbol_type; // symbol type
};

}