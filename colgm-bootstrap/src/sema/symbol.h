#pragma once

#include "colgm.h"

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
    func_kind
};

struct type {
    std::string name;
    u64 pointer_level;
    bool is_global = false;

    std::string to_string() const;
    bool operator==(const type&) const;
    bool operator!=(const type&) const;
    friend std::ostream& operator<<(std::ostream&, const type&);
    static type error_type() { return {"<err>", 0}; }

    bool is_error() const { return name=="<err>"; }
};

struct symbol {
    std::string name;
    type symbol_type;
};

struct colgm_func {
    std::string name;
    type return_type;
    std::vector<symbol> parameters;

    bool find_parameter(const std::string&);
    void add_parameter(const std::string&, const type&);
};

struct colgm_struct {
    std::string name;
    std::vector<symbol> field;
    std::unordered_map<std::string, colgm_func> method;
};

struct semantic_context {
    std::unordered_set<std::string> constant_string;
    std::unordered_map<std::string, colgm_struct> structs;
    std::unordered_map<std::string, colgm_func> functions;
    std::unordered_map<std::string, symbol_kind> symbols;
    std::vector<std::unordered_map<std::string, type>> scope = {};

    bool find_symbol(const std::string&);
    void add_symbol(const std::string&, const type&);
    void push_new_level() { scope.push_back({}); }
    void pop_new_level() { scope.pop_back(); }
};

}