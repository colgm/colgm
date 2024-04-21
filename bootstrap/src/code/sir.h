#pragma once

#include "colgm.h"
#include "code/value.h"

#include <cstdint>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace colgm {

enum class sir_kind {
    sir_null = 0,
    sir_nop,
    sir_block,
    sir_alloca,
    sir_nil,
    sir_number,
    sir_temp_ptr,
    sir_ret,
    sir_str,
    sir_call_index,
    sir_call_field,
    sir_call_func,
    sir_neg,
    sir_bnot,
    sir_binary,
    sir_label,
    sir_place_holder_label,
    sir_store,
    sir_store_literal,
    sir_load,
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

class sir_nop: public sir {
private:
    std::string info;

public:
    sir_nop(const std::string& i): sir(sir_kind::sir_nop), info(i) {}
    ~sir_nop() override = default;
    void dump(std::ostream& os) const override {
        if (info.empty()) {
            os << "\n";
            return;
        }
        os << "; " << info << "\n";
    }
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

class sir_nil: public sir {
private:
    std::string destination;

public:
    sir_nil(const std::string& dst):
        sir(sir_kind::sir_nil), destination(dst) {}
    ~sir_nil() override = default;
    void dump(std::ostream&) const override;
};

class sir_number: public sir {
private:
    std::string literal;
    std::string destination;
    std::string type_name;
    bool is_integer;

public:
    sir_number(const std::string& lt,
               const std::string& dst,
               const std::string& type,
               bool ii):
        sir(sir_kind::sir_number), literal(lt),
        destination(dst), type_name(type),
        is_integer(ii) {}
    ~sir_number() override = default;
    void dump(std::ostream&) const override;
};

class sir_temp_ptr: public sir {
/*
used to get temporary pointer of numbering variable
    %real.1 = alloca i32
this operand will do this:
    %1 = getelementptr i32, i32* %real.1, i32 0
and %1 is i32*
*/
private:
    std::string temp_name;
    std::string type_name;

public:
    sir_temp_ptr(const std::string& tmp, const std::string type):
        sir(sir_kind::sir_temp_ptr), temp_name(tmp), type_name(type) {}
    ~sir_temp_ptr() override = default;
    void dump(std::ostream&) const override;
};

class sir_block: public sir {
private:
    std::vector<sir_alloca*> allocas;
    std::vector<sir*> stmts;

public:
    sir_block(): sir(sir_kind::sir_block) {}
    ~sir_block() override;
    void dump(std::ostream&) const override;

    void add_alloca(sir_alloca* node) { allocas.push_back(node); }
    void add_stmt(sir* node) { stmts.push_back(node); }
    void add_nop(const std::string& info = "") {
        stmts.push_back(new sir_nop(info));
    }
    auto stmt_size() const { return stmts.size(); }
    bool back_is_ret_stmt() const {
        return stmts.size() && stmts.back()->get_ir_type()==sir_kind::sir_ret;
    }
};

class sir_ret: public sir {
private:
    std::string type;
    std::string value;

public:
    sir_ret(const std::string& t, const std::string& v = ""):
        sir(sir_kind::sir_ret), type(t), value(v) {}
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

class sir_call_index: public sir {
private:
    std::string source;
    std::string destination;
    std::string index;
    std::string type;
    std::string index_type;

public:
    sir_call_index(const std::string& src,
                   const std::string& dst,
                   const std::string& idx,
                   const std::string& t,
                   const std::string& it):
        sir(sir_kind::sir_call_index), source(src),
        destination(dst), index(idx), type(t), index_type(it) {}
    ~sir_call_index() override = default;
    void dump(std::ostream&) const override;
};

class sir_call_field: public sir {
private:
    std::string destination;
    std::string source;
    std::string struct_name;
    usize index;

public:
    sir_call_field(const std::string& dst,
                   const std::string& src,
                   const std::string& sn,
                   usize i):
        sir(sir_kind::sir_call_field),
        destination(dst), source(src),
        struct_name(sn), index(i) {}
    ~sir_call_field() override = default;
    void dump(std::ostream&) const override;
};

class sir_call_func: public sir {
private:
    std::string name;
    std::string return_type;
    std::string destination;
    std::vector<std::string> args_type;
    std::vector<std::string> args;

public:
    sir_call_func(const std::string& n,
                  const std::string& rt,
                  const std::string& dst):
        sir(sir_kind::sir_call_func), name(n),
        return_type(rt), destination(dst) {}
    ~sir_call_func() override = default;
    void add_arg_type(const std::string& t) { args_type.push_back(t); }
    void add_arg(const std::string& a) { args.push_back(a); }
    void dump(std::ostream&) const override;
};

class sir_neg: public sir {
private:
    std::string source;
    std::string destination;
    std::string opr;
    std::string type;

public:
    sir_neg(const std::string& src,
            const std::string& dst,
            const std::string& o,
            const std::string& t):
        sir(sir_kind::sir_neg),
        source(src), destination(dst),
        opr(o), type(t) {}
    ~sir_neg() override = default;
    void dump(std::ostream&) const override;
};

class sir_bnot: public sir {
private:
    std::string source;
    std::string destination;
    std::string type;

public:
    sir_bnot(const std::string& src,
             const std::string& dst,
             const std::string& t):
        sir(sir_kind::sir_bnot),
        source(src), destination(dst),
        type(t) {}
    ~sir_bnot() override = default;
    void dump(std::ostream&) const override;
};

class sir_binary: public sir {
private:
    std::string left;
    std::string right;
    std::string destination;
    std::string opr;
    std::string type;

public:
    sir_binary(const std::string& l,
               const std::string& r,
               const std::string& dst,
               const std::string& o,
               const std::string& t):
        sir(sir_kind::sir_binary),
        left(l), right(r),
        destination(dst),
        opr(o), type(t) {}
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

class sir_place_holder_label: public sir {
private:
    usize label_count;

public:
    sir_place_holder_label(usize count):
        sir(sir_kind::sir_place_holder_label), label_count(count) {}
    ~sir_place_holder_label() override = default;
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

class sir_store_literal: public sir {
private:
    std::string type;
    std::string source;
    std::string destination;

public:
    sir_store_literal(const std::string& t,
                      const std::string& src,
                      const std::string& dst):
        sir(sir_kind::sir_store_literal), type(t), source(src), destination(dst) {}
    ~sir_store_literal() override = default;
    void dump(std::ostream&) const override;
};

class sir_load: public sir {
private:
    std::string type;
    std::string source;
    std::string destination;

public:
    sir_load(const std::string& t,
             const std::string& src,
             const std::string& dst):
        sir(sir_kind::sir_load), type(t), source(src), destination(dst) {}
    ~sir_load() override = default;
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

    void set_label(usize dst) { destination = dst; }
};

class sir_br_cond: public sir {
private:
    std::string condition;
    usize destination_true;
    usize destination_false;

public:
    sir_br_cond(const std::string& cd, u64 dst_true, u64 dst_false):
        sir(sir_kind::sir_br_cond),
        condition(cd),
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