#pragma once

#include <ostream>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace colgm {

enum class symbol_kind {
    basic_kind,
    struct_kind,
    func_kind
};

struct type {
    std::string name;
    uint64_t pointer_level;

    std::string to_string() const;
    friend std::ostream& operator<<(std::ostream&, const type&);
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