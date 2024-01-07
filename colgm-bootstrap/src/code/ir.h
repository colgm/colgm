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
    cir_func_decl,
    cir_code_block,
    cir_ret,
    cir_def,
    cir_num,
    cir_str,
    cir_get_var,
    cir_call_index,
    cir_call_field,
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

class ir_code_block: public ir {
private:
    std::vector<ir*> stmts;

public:
    ir_code_block(): ir(ir_kind::cir_code_block) {}
    ~ir_code_block() override;
    void dump(std::ostream&) override;

    void add_stmt(ir* node) { stmts.push_back(node); }
};

class ir_ret: public ir {
private:
    bool has_return_value;

public:
    ir_ret(bool rv_flag):
        ir(ir_kind::cir_ret), has_return_value(rv_flag) {}
    ~ir_ret() override = default;
    void dump(std::ostream&) override;
};

class ir_def: public ir {
private:
    std::string name;
    std::string type;

public:
    ir_def(const std::string& n, const std::string& t):
        ir(ir_kind::cir_def), name(n), type(t) {}
    ~ir_def() override = default;
    void dump(std::ostream&) override;
};

class ir_func_decl: public ir {
private:
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::string return_type;
    ir_code_block* cb;

public:
    ir_func_decl(const std::string& n):
        ir(ir_kind::cir_func_decl), name(n), cb(nullptr) {}
    ~ir_func_decl() override;
    void dump(std::ostream&) override;

    void add_param(const std::string& pname, const std::string& ptype) {
        params.push_back({pname, ptype});
    }
    void set_code_block(ir_code_block* node) { cb = node; }
    auto get_code_block() { return cb; }
    void set_return_type(const std::string& rtype) { return_type = rtype; }
};

class ir_number: public ir {
private:
    std::string literal;

public:
    ir_number(const std::string& n):
        ir(ir_kind::cir_num), literal(n) {}
    ~ir_number() override = default;
    void dump(std::ostream&) override;
};

class ir_string: public ir {
private:
    std::string literal;

public:
    ir_string(const std::string& s):
        ir(ir_kind::cir_str), literal(s) {}
    ~ir_string() override = default;
    void dump(std::ostream&) override;
};

class ir_get_var: public ir {
private:
    std::string name;

public:
    ir_get_var(const std::string& n):
        ir(ir_kind::cir_get_var), name(n) {}
    ~ir_get_var() override = default;
    void dump(std::ostream&) override;
};

class ir_call_index: public ir {
public:
    ir_call_index(): ir(ir_kind::cir_call_index) {}
    ~ir_call_index() override = default;
    void dump(std::ostream&) override;
};

class ir_call_field: public ir {
private:
    std::string field_name;

public:
    ir_call_field(const std::string& n):
        ir(ir_kind::cir_call_field), field_name(n) {}
    ~ir_call_field() override = default;
    void dump(std::ostream&) override;
};

class ir_call_func: public ir {
private:
    size_t argc;

public:
    ir_call_func(size_t ac):
        ir(ir_kind::cir_call_func), argc(ac) {}
    ~ir_call_func() override = default;
    void dump(std::ostream&) override;
};

class ir_binary: public ir {
private:
    std::string opr;

public:
    ir_binary(const std::string& o):
        ir(ir_kind::cir_binary), opr(o) {}
    ~ir_binary() override = default;
    void dump(std::ostream&) override;
};

class ir_label: public ir {
private:
    uint64_t label_count;

public:
    ir_label(uint64_t count):
        ir(ir_kind::cir_label), label_count(count) {}
    ~ir_label() override = default;
    void dump(std::ostream&) override;
};

class ir_assign: public ir {
private:
    std::string opr;

public:
    ir_assign(const std::string& o):
        ir(ir_kind::cir_assign), opr(o) {}
    ~ir_assign() override = default;
    void dump(std::ostream&) override;
};

class ir_br_direct: public ir {
private:
    uint64_t destination;

public:
    ir_br_direct(uint64_t dst):
        ir(ir_kind::cir_br_direct), destination(dst) {}
    ~ir_br_direct() override = default;
    void dump(std::ostream&) override;
};

class ir_br_cond: public ir {
private:
    uint64_t destination_true;
    uint64_t destination_false;

public:
    ir_br_cond(uint64_t dst_true, uint64_t dst_false):
        ir(ir_kind::cir_br_cond),
        destination_true(dst_true),
        destination_false(dst_false) {}
    ~ir_br_cond() override = default;
    void dump(std::ostream&) override;

    void set_false_label(uint64_t dst_false) {
        destination_false = dst_false;
    }
};

}