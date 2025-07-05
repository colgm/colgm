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
    tagged_union_kind,
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
    bool is_global = false;      // global symbol
    bool is_global_func = false; // global func
    bool is_enum = false;        // enum type
    bool is_const = false;       // const type
    bool is_generic_placeholder = false; // generic placeholder

    u64 array_length = 0;        // array length
    bool is_array = false;       // array type

    struct_method_info stm_info; // struct methods
    struct_method_info prm_info; // primitive methods
    std::vector<type> generics = {};

private:
    void check_pointer_depth() const;

public:
    std::string to_string() const;
    std::string array_type_to_string() const;
    std::string full_path_name(bool with_generics = true) const;
    std::string full_path_name_with_pointer(bool with_generics = true) const;
    std::string generic_name() const;
    u64 generic_depth() const;
    // return name for symbol table search
    // normal type: .name
    // generic type: .generic_name()
    std::string name_for_search() const {
        return generics.size() ? generic_name() : name;
    }
    bool operator==(const type& rhs) const {
        return name == rhs.name &&
               pointer_depth == rhs.pointer_depth &&
               generics == rhs.generics;
    }
    bool operator!=(const type& rhs) const {
        return name != rhs.name ||
               pointer_depth != rhs.pointer_depth ||
               generics != rhs.generics;
    }
    bool eq_no_ptr(const type& rhs) const {
        return name == rhs.name &&
               generics == rhs.generics;
    }
    friend std::ostream& operator<<(std::ostream&, const type&);

public:
    static const type error_type() { return {"<err>", "", 0}; }
    static const type restrict_type() { return {"<restrict>", "", 0}; }
    static const type default_match_type() { return {"<match default>", "", 0}; }
    static const type empty_array_type() {
        auto res = type {"<empty array>", "", 0};
        res.is_array = true;
        res.array_length = 0;
        return res;
    }
    static const type u64_type(i64 plvl = 0) { return {"u64", "", plvl}; }
    static const type u32_type(i64 plvl = 0) { return {"u32", "", plvl}; }
    static const type u16_type(i64 plvl = 0) { return {"u16", "", plvl}; }
    static const type u8_type(i64 plvl = 0) { return {"u8", "", plvl}; }
    static const type i64_type(i64 plvl = 0) { return {"i64", "", plvl}; }
    static const type i32_type(i64 plvl = 0) { return {"i32", "", plvl}; }
    static const type i16_type(i64 plvl = 0) { return {"i16", "", plvl}; }
    static const type i8_type(i64 plvl = 0) { return {"i8", "", plvl}; }
    static const type f64_type(i64 plvl = 0) { return {"f64", "", plvl}; }
    static const type f32_type(i64 plvl = 0) { return {"f32", "", plvl}; }
    static const type void_type(i64 plvl = 0) { return {"void", "", plvl}; }
    static const type bool_type(i64 plvl = 0) { return {"bool", "", plvl}; }

    static const type const_str_literal_type() {
        auto t = type::i8_type(1);
        t.is_const = true;
        return t;
    }

public:
    bool is_empty() const { return name.empty() && loc_file.empty(); }
    bool is_error() const { return name=="<err>"; }
    bool is_restrict() const { return name=="<restrict>"; }
    bool is_default_match() const { return name=="<match default>"; }
    bool is_empty_array() const { return name=="<empty array>"; }
    bool is_unsigned() const {
        const auto& t = *this;
        return t == type::u8_type() ||
               t == type::u16_type() ||
               t == type::u32_type() ||
               t == type::u64_type() ||
               t.pointer_depth > 0;
    }
    bool is_integer() const {
        const auto& t = *this;
        return t == type::i8_type(t.pointer_depth) ||
               t == type::u8_type(t.pointer_depth) ||
               t == type::i16_type(t.pointer_depth) ||
               t == type::u16_type(t.pointer_depth) ||
               t == type::i32_type(t.pointer_depth) ||
               t == type::u32_type(t.pointer_depth) ||
               t == type::i64_type(t.pointer_depth) ||
               t == type::u64_type(t.pointer_depth) ||
               (t.is_enum && !t.pointer_depth);
    }
    bool is_float() const {
        const auto& t = *this;
        return t == type::f32_type() ||
               t == type::f64_type();
    }
    bool is_boolean() const {
        return *this == type::bool_type(pointer_depth);
    }
    bool can_bitwise_calculate() const { 
        return (is_integer() || is_boolean()) && !pointer_depth;
    }
    bool is_void() const { return *this == type::void_type(0); }
    bool is_pointer() const { return pointer_depth > 0; }
    bool is_function() const {
        return stm_info.flag_is_normal ||
               stm_info.flag_is_static ||
               prm_info.flag_is_normal ||
               prm_info.flag_is_static ||
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
    void dump(const std::string& end = "\n") const;
};

}