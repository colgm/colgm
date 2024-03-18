#pragma once

#include "colgm.h"
#include "err.h"

#include <ostream>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace colgm {

enum class symbol_kind {
    basic_kind,
    struct_kind,
    func_kind,
    package_kind,
    module_kind
};

struct struct_method {
    bool flag_is_static = false; // mark static method
    bool flag_is_normal = false; // mark normal method
    std::string method_name;
};

struct type {
    std::string name;
    std::string loc_file;
    u64 pointer_level = 0;
    bool is_global = false;
    bool is_global_func = false;

    struct_method stm_info;

    std::string to_string() const;
    bool operator==(const type& another) const {
        return name==another.name &&
               pointer_level==another.pointer_level;
    }
    bool operator!=(const type& another) const {
        return name!=another.name ||
               pointer_level!=another.pointer_level;
    }
    friend std::ostream& operator<<(std::ostream&, const type&);
    
    static const type error_type() { return {"<err>", "", 0}; }
    static const type u64_type(u64 ptrlvl = 0) { return {"u64", "", ptrlvl}; }
    static const type u32_type(u64 ptrlvl = 0) { return {"u32", "", ptrlvl}; }
    static const type u16_type(u64 ptrlvl = 0) { return {"u16", "", ptrlvl}; }
    static const type u8_type(u64 ptrlvl = 0) { return {"u8", "", ptrlvl}; }
    static const type i64_type(u64 ptrlvl = 0) { return {"i64", "", ptrlvl}; }
    static const type i32_type(u64 ptrlvl = 0) { return {"i32", "", ptrlvl}; }
    static const type i16_type(u64 ptrlvl = 0) { return {"i16", "", ptrlvl}; }
    static const type i8_type(u64 ptrlvl = 0) { return {"i8", "", ptrlvl}; }
    static const type f64_type(u64 ptrlvl = 0) { return {"f64", "", ptrlvl}; }
    static const type f32_type(u64 ptrlvl = 0) { return {"f32", "", ptrlvl}; }
    static const type void_type(u64 ptrlvl = 0) { return {"void", "", ptrlvl}; }
    static const type bool_type(u64 ptrlvl = 0) { return {"bool", "", ptrlvl}; }

    bool is_error() const { return name=="<err>"; }
    bool is_integer() const {
        const auto& t = *this;
        return t==type::i8_type() || t==type::u8_type() ||
               t==type::i16_type() || t==type::u16_type() ||
               t==type::i32_type() || t==type::u32_type() ||
               t==type::i64_type() || t==type::u64_type();
    }
    bool is_float() const {
        const auto& t = *this;
        return t==type::f32_type() || t==type::f64_type();
    }
    bool is_boolean() const {
        return *this==type::bool_type();
    }
    bool is_pointer() const { return pointer_level; }
};

struct symbol {
    std::string name;
    type symbol_type;
};

struct colgm_func {
    std::string name;
    span location;
    type return_type;
    std::vector<symbol> parameters;
    std::unordered_map<std::string, symbol> unordered_params;

    bool find_parameter(const std::string&);
    void add_parameter(const std::string&, const type&);
};

struct colgm_struct {
    std::string name;
    span location;
    std::unordered_map<std::string, symbol> field;
    std::vector<symbol> ordered_field;
    std::unordered_map<std::string, colgm_func> static_method;
    std::unordered_map<std::string, colgm_func> method;
};

}