#pragma once

#include "colgm.h"
#include "code/sir.h"

namespace colgm {

class hir_struct {
private:
    std::string name;
    std::vector<std::string> field_type;

public:
    hir_struct(const std::string& n): name(n) {}
    void dump(std::ostream&) const;

    void add_field_type(const std::string& type) { field_type.push_back(type); }
};

class hir_func {
private:
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::string return_type;
    sir_code_block* alloca_block;
    sir_code_block* cb;

public:
    hir_func(const std::string& n):
        name(n), alloca_block(nullptr), cb(nullptr) {}
    ~hir_func();
    void dump(std::ostream&) const;

    void add_param(const std::string& pname, const std::string& ptype) {
        params.push_back({pname, ptype});
    }
    void set_alloca_block(sir_code_block* b) { alloca_block = b; }
    auto get_alloca_block() { return alloca_block; }
    void set_code_block(sir_code_block* b) { cb = b; }
    auto get_code_block() { return cb; }
    void set_return_type(const std::string& rtype) { return_type = rtype; }
};

}