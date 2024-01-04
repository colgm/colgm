#pragma once

#include <cstdint>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace colgm {

enum class ir_kind {
    op_struct,
    op_func_decl,
    op_func_param_name,
    op_func_param_type
};

class ir {
private:
    ir_kind type;

public:
    ir(ir_kind t): type(t) {}
    virtual ~ir() = default;
    virtual void dump(std::ostream&) = 0;
};

class ir_struct: public ir {
private:
    std::string name;
    std::vector<std::string> field_type;

public:
    ir_struct(const std::string& n):
        ir(ir_kind::op_struct), name(n) {}
    ~ir_struct() override = default;
    void dump(std::ostream&) override;

    void add_field_type(const std::string& type) { field_type.push_back(type); }
};

}