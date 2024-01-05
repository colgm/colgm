#pragma once

#include <cstdint>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace colgm {

enum class ir_kind {
    cir_struct,
    cir_func_decl
};

class ir {
private:
    ir_kind type;

public:
    ir(ir_kind k): type(k) {}
    virtual ~ir() = default;
    virtual void dump(std::ostream&) = 0;
    auto get_ir_type() const { return type; }
};

class ir_struct: public ir {
private:
    std::string name;
    std::vector<std::string> field_type;

public:
    ir_struct(const std::string& n):
        ir(ir_kind::cir_struct), name(n) {}
    ~ir_struct() override = default;
    void dump(std::ostream&) override;

    void add_field_type(const std::string& type) { field_type.push_back(type); }
};

class ir_func_decl: public ir {
private:
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::string return_type;

public:
    ir_func_decl(const std::string& n):
        ir(ir_kind::cir_func_decl), name(n) {}
    ~ir_func_decl() override = default;
    void dump(std::ostream&) override;

    void add_param(const std::string& pname, const std::string& ptype) {
        params.push_back({pname, ptype});
    }
    void set_return_type(const std::string& rtype) { return_type = rtype; }
};

}