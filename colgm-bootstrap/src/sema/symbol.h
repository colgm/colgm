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
    u64 pointer_level;
    bool is_global = false;
    bool is_global_func = false;

    struct_method stm_info;

    std::string to_string() const;
    bool operator==(const type&) const;
    bool operator!=(const type&) const;
    friend std::ostream& operator<<(std::ostream&, const type&);
    
    static const type error_type() { return {"<err>", 0}; }
    static const type u64_type(u64 ptrlvl = 0) { return {"u64", ptrlvl}; }
    static const type u32_type(u64 ptrlvl = 0) { return {"u32", ptrlvl}; }
    static const type u16_type(u64 ptrlvl = 0) { return {"u16", ptrlvl}; }
    static const type u8_type(u64 ptrlvl = 0) { return {"u8", ptrlvl}; }
    static const type i64_type(u64 ptrlvl = 0) { return {"i64", ptrlvl}; }
    static const type i32_type(u64 ptrlvl = 0) { return {"i32", ptrlvl}; }
    static const type i16_type(u64 ptrlvl = 0) { return {"i16", ptrlvl}; }
    static const type i8_type(u64 ptrlvl = 0) { return {"i8", ptrlvl}; }
    static const type f64_type(u64 ptrlvl = 0) { return {"f64", ptrlvl}; }
    static const type f32_type(u64 ptrlvl = 0) { return {"f32", ptrlvl}; }
    static const type void_type(u64 ptrlvl = 0) { return {"void", ptrlvl}; }
    static const type bool_type(u64 ptrlvl = 0) { return {"bool", ptrlvl}; }

    bool is_error() const { return name=="<err>"; }
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

    bool find_parameter(const std::string&);
    void add_parameter(const std::string&, const type&);
};

struct colgm_struct {
    std::string name;
    span location;
    std::vector<symbol> field;
    std::unordered_map<std::string, colgm_func> method;
};

struct semantic_context {
    std::unordered_set<std::string> constant_string;
    std::unordered_map<std::string, colgm_struct> structs;
    std::unordered_map<std::string, colgm_func> functions;
    std::unordered_map<std::string, symbol_kind> global_symbol;
    std::vector<std::unordered_map<std::string, type>> scope = {};

    bool find_symbol(const std::string&);
    void add_symbol(const std::string&, const type&);
    type get_symbol(const std::string&);
    void push_new_level() { scope.push_back({}); }
    void pop_new_level() { scope.pop_back(); }

    void dump_structs() const;
    void dump_functions() const;
    void dump_single_function(const colgm_func&,
                              const std::string& indent = "") const;
};

}