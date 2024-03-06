#pragma once

#include "colgm.h"

#include <cstdint>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace colgm {

enum class ir_kind {
    cir_code_block,
    cir_alloca,
    cir_ret,
    cir_num,
    cir_str,
    cir_bool,
    cir_get_var,
    cir_call_index,
    cir_call_field,
    cir_ptr_call_field,
    cir_call_path,
    cir_call_func,
    cir_binary,
    cir_label,
    cir_assign,
    cir_br_direct,
    cir_br_cond
};

class ir {
private:
    ir_kind type;

public:
    ir(ir_kind k): type(k) {}
    virtual ~ir() = default;
    virtual void dump(std::ostream&) const = 0;
    auto get_ir_type() const { return type; }
};

class ir_alloca: public ir {
private:
    std::string variable_name;
    std::string type_name;

public:
    ir_alloca(const std::string& v, const std::string& t):
        ir(ir_kind::cir_alloca), variable_name(v), type_name(t) {}
    ~ir_alloca() override = default;
    void dump(std::ostream&) const override;
};

class ir_code_block: public ir {
private:
    std::vector<ir*> stmts;

public:
    ir_code_block(): ir(ir_kind::cir_code_block) {}
    ~ir_code_block() override;
    void dump(std::ostream&) const override;

    void add_stmt(ir* node) { stmts.push_back(node); }
    auto size() const { return stmts.size(); }
};

class ir_ret: public ir {
private:
    bool has_return_value;

public:
    ir_ret(bool rv_flag):
        ir(ir_kind::cir_ret), has_return_value(rv_flag) {}
    ~ir_ret() override = default;
    void dump(std::ostream&) const override;
};

class ir_number: public ir {
private:
    std::string literal;

public:
    ir_number(const std::string& n):
        ir(ir_kind::cir_num), literal(n) {}
    ~ir_number() override = default;
    void dump(std::ostream&) const override;
};

class ir_string: public ir {
private:
    std::string literal;

public:
    ir_string(const std::string& s):
        ir(ir_kind::cir_str), literal(s) {}
    ~ir_string() override = default;
    void dump(std::ostream&) const override;
};

class ir_bool: public ir {
private:
    bool flag;

public:
    ir_bool(bool f): ir(ir_kind::cir_bool), flag(f) {}
    ~ir_bool() override = default;
    void dump(std::ostream&) const override;
};

class ir_get_var: public ir {
private:
    std::string name;

public:
    ir_get_var(const std::string& n):
        ir(ir_kind::cir_get_var), name(n) {}
    ~ir_get_var() override = default;
    void dump(std::ostream&) const override;
};

class ir_call_index: public ir {
public:
    ir_call_index(): ir(ir_kind::cir_call_index) {}
    ~ir_call_index() override = default;
    void dump(std::ostream&) const override;
};

class ir_call_field: public ir {
private:
    std::string field_name;

public:
    ir_call_field(const std::string& n):
        ir(ir_kind::cir_call_field), field_name(n) {}
    ~ir_call_field() override = default;
    void dump(std::ostream&) const override;
};

class ir_ptr_call_field: public ir {
private:
    std::string field_name;

public:
    ir_ptr_call_field(const std::string& n):
        ir(ir_kind::cir_ptr_call_field), field_name(n) {}
    ~ir_ptr_call_field() override = default;
    void dump(std::ostream&) const override;
};

class ir_call_path: public ir {
private:
    std::string field_name;

public:
    ir_call_path(const std::string& n):
        ir(ir_kind::cir_call_path), field_name(n) {}
    ~ir_call_path() override = default;
    void dump(std::ostream&) const override;
};

class ir_call_func: public ir {
private:
    usize argc;

public:
    ir_call_func(usize ac):
        ir(ir_kind::cir_call_func), argc(ac) {}
    ~ir_call_func() override = default;
    void dump(std::ostream&) const override;
};

class ir_binary: public ir {
private:
    std::string opr;

public:
    ir_binary(const std::string& o):
        ir(ir_kind::cir_binary), opr(o) {}
    ~ir_binary() override = default;
    void dump(std::ostream&) const override;
};

class ir_label: public ir {
private:
    usize label_count;

public:
    ir_label(usize count):
        ir(ir_kind::cir_label), label_count(count) {}
    ~ir_label() override = default;
    void dump(std::ostream&) const override;
};

class ir_assign: public ir {
private:
    std::string opr;

public:
    ir_assign(const std::string& o):
        ir(ir_kind::cir_assign), opr(o) {}
    ~ir_assign() override = default;
    void dump(std::ostream&) const override;
};

class ir_br_direct: public ir {
private:
    usize destination;

public:
    ir_br_direct(usize dst):
        ir(ir_kind::cir_br_direct), destination(dst) {}
    ~ir_br_direct() override = default;
    void dump(std::ostream&) const override;
};

class ir_br_cond: public ir {
private:
    usize destination_true;
    usize destination_false;

public:
    ir_br_cond(u64 dst_true, u64 dst_false):
        ir(ir_kind::cir_br_cond),
        destination_true(dst_true),
        destination_false(dst_false) {}
    ~ir_br_cond() override = default;
    void dump(std::ostream&) const override;

    void set_true_label(u64 dst_true) {
        destination_true = dst_true;
    }
    void set_false_label(u64 dst_false) {
        destination_false = dst_false;
    }
};

}