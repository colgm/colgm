#pragma once

#include "colgm.h"
#include "code/sir.h"

namespace colgm {

class hir_struct {
private:
    std::string name;
    span location;
    std::vector<std::string> field_type;

public:
    hir_struct(const std::string& n, const span& loc): name(n), location(loc) {}
    void dump(std::ostream&) const;
    const auto& get_name() const { return name; }
    const auto& get_location() const { return location; }

    void add_field_type(const std::string& type) { field_type.push_back(type); }
};

class hir_func {
private:
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::string return_type;
    sir_block* cb;

public:
    hir_func(const std::string& n):
        name(n), cb(nullptr) {}
    ~hir_func();
    void dump(std::ostream&) const;
    const auto& get_name() const { return name; }

    void add_param(const std::string& pname, const std::string& ptype) {
        params.push_back({pname, ptype});
    }
    void set_code_block(sir_block* b) { cb = b; }
    auto get_code_block() { return cb; }
    void set_return_type(const std::string& rtype) { return_type = rtype; }
};

}