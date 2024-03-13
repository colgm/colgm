#pragma once

#include "colgm.h"

#include <cstdint>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace colgm {

enum class sir_kind {
    sir_code_block,
    sir_alloca,
    sir_ret,
    sir_str,
    sir_get_var,
    sir_call_index,
    sir_call_field,
    sir_ptr_call_field,
    sir_call_path,
    sir_call_func,
    sir_binary,
    sir_label,
    sir_assign,
    sir_store,
    sir_br_direct,
    sir_br_cond
};

class sir {
private:
    sir_kind type;

public:
    sir(sir_kind k): type(k) {}
    virtual ~sir() = default;
    virtual void dump(std::ostream&) const = 0;
    auto get_ir_type() const { return type; }
};

class sir_alloca: public sir {
private:
    std::string variable_name;
    std::string type_name;

public:
    sir_alloca(const std::string& v, const std::string& t):
        sir(sir_kind::sir_alloca), variable_name(v), type_name(t) {}
    ~sir_alloca() override = default;
    void dump(std::ostream&) const override;
};

class sir_code_block: public sir {
private:
    std::vector<sir*> stmts;

public:
    sir_code_block(): sir(sir_kind::sir_code_block) {}
    ~sir_code_block() override;
    void dump(std::ostream&) const override;

    void add_stmt(sir* node) { stmts.push_back(node); }
    auto size() const { return stmts.size(); }
};

class sir_ret: public sir {
private:
    bool has_return_value;

public:
    sir_ret(bool rv_flag):
        sir(sir_kind::sir_ret), has_return_value(rv_flag) {}
    ~sir_ret() override = default;
    void dump(std::ostream&) const override;
};

class sir_string: public sir {
private:
    usize index;
    std::string literal;
    std::string destination;

public:
    sir_string(const std::string& s, const usize i, const std::string& dst):
        sir(sir_kind::sir_str), index(i), literal(s), destination(dst) {}
    ~sir_string() override = default;
    void dump(std::ostream&) const override;
};

class sir_get_var: public sir {
private:
    std::string name;

public:
    sir_get_var(const std::string& n):
        sir(sir_kind::sir_get_var), name(n) {}
    ~sir_get_var() override = default;
    void dump(std::ostream&) const override;
};

class sir_call_index: public sir {
private:
    std::string source;
    std::string destination;
    std::string index;

public:
    sir_call_index(const std::string& src,
                   const std::string& dst,
                   const std::string& idx):
        sir(sir_kind::sir_call_index), source(src),
        destination(dst), index(idx) {}
    ~sir_call_index() override = default;
    void dump(std::ostream&) const override;
};

class sir_call_field: public sir {
private:
    std::string field_name;

public:
    sir_call_field(const std::string& n):
        sir(sir_kind::sir_call_field), field_name(n) {}
    ~sir_call_field() override = default;
    void dump(std::ostream&) const override;
};

class sir_ptr_call_field: public sir {
private:
    std::string field_name;

public:
    sir_ptr_call_field(const std::string& n):
        sir(sir_kind::sir_ptr_call_field), field_name(n) {}
    ~sir_ptr_call_field() override = default;
    void dump(std::ostream&) const override;
};

class sir_call_path: public sir {
private:
    std::string field_name;

public:
    sir_call_path(const std::string& n):
        sir(sir_kind::sir_call_path), field_name(n) {}
    ~sir_call_path() override = default;
    void dump(std::ostream&) const override;
};

class sir_call_func: public sir {
private:
    usize argc;

public:
    sir_call_func(usize ac):
        sir(sir_kind::sir_call_func), argc(ac) {}
    ~sir_call_func() override = default;
    void dump(std::ostream&) const override;
};

class sir_binary: public sir {
private:
    std::string opr;

public:
    sir_binary(const std::string& o):
        sir(sir_kind::sir_binary), opr(o) {}
    ~sir_binary() override = default;
    void dump(std::ostream&) const override;
};

class sir_label: public sir {
private:
    usize label_count;

public:
    sir_label(usize count):
        sir(sir_kind::sir_label), label_count(count) {}
    ~sir_label() override = default;
    void dump(std::ostream&) const override;
};

class sir_assign: public sir {
private:
    std::string opr;

public:
    sir_assign(const std::string& o):
        sir(sir_kind::sir_assign), opr(o) {}
    ~sir_assign() override = default;
    void dump(std::ostream&) const override;
};

class sir_store: public sir {
private:
    std::string type;
    std::string source;
    std::string destination;

public:
    sir_store(const std::string& t,
              const std::string& src,
              const std::string& dst):
        sir(sir_kind::sir_store), type(t), source(src), destination(dst) {}
    ~sir_store() override = default;
    void dump(std::ostream&) const override;
};

class sir_br: public sir {
private:
    usize destination;

public:
    sir_br(usize dst):
        sir(sir_kind::sir_br_direct), destination(dst) {}
    ~sir_br() override = default;
    void dump(std::ostream&) const override;
};

class sir_br_cond: public sir {
private:
    usize destination_true;
    usize destination_false;

public:
    sir_br_cond(u64 dst_true, u64 dst_false):
        sir(sir_kind::sir_br_cond),
        destination_true(dst_true),
        destination_false(dst_false) {}
    ~sir_br_cond() override = default;
    void dump(std::ostream&) const override;

    void set_true_label(u64 dst_true) {
        destination_true = dst_true;
    }
    void set_false_label(u64 dst_false) {
        destination_false = dst_false;
    }
};

}