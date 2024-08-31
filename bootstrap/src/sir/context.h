#pragma once 

#include "colgm.h"
#include "sema/symbol.h"
#include "sir/sir.h"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <sstream>

namespace colgm {

class sir_struct {
private:
    std::string name;
    span location;
    std::vector<std::string> field_type;

public:
    sir_struct(const std::string& n, const span& loc): name(n), location(loc) {}
    void dump(std::ostream&) const;
    const auto& get_name() const { return name; }
    const auto& get_location() const { return location; }

    void add_field_type(const std::string& type) { field_type.push_back(type); }
    const auto& get_field_type() const { return field_type; }
};

class sir_func {
private:
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::string return_type;
    sir_block* block;

public:
    sir_func(const std::string& n):
        name(n), block(nullptr) {}
    ~sir_func() { delete block; }
    void dump(std::ostream&) const;
    const auto& get_name() const { return name; }

    void add_param(const std::string& pname, const std::string& ptype) {
        params.push_back({pname, ptype});
    }
    void set_code_block(sir_block* b) { block = b; }
    auto get_code_block() { return block; }
    void set_return_type(const std::string& rtype) { return_type = rtype; }
};

struct sir_context {
    std::vector<sir_struct*> struct_decls;
    std::vector<sir_func*> func_decls;
    std::vector<sir_func*> func_impls;
    std::unordered_map<std::string, u64> const_strings;

private:
    bool passes_already_executed = false;

private:
    void dump_const_string(std::ostream&) const;
    void dump_struct_size_method(std::ostream&) const;
    void dump_struct_alloc_method(std::ostream&) const;

public:
    ~sir_context() {
        for(auto i : struct_decls) {
            delete i;
        }
        for(auto i : func_decls) {
            delete i;
        }
        for(auto i : func_impls) {
            delete i;
        }
    }
    void dump_code(std::ostream&);
};

}