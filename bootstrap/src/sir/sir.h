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
    sir_null = 0,
    sir_nop,
    sir_block,
    sir_alloca,
    sir_temp_ptr,
    sir_ret,
    sir_str,
    sir_zeroinitializer,
    sir_call_index,
    sir_call_field,
    sir_call_func,
    sir_neg,
    sir_bnot,
    sir_lnot,
    sir_add,
    sir_sub,
    sir_mul,
    sir_div,
    sir_rem,
    sir_band,
    sir_bxor,
    sir_bor,
    sir_cmp,
    sir_label,
    sir_place_holder_label,
    sir_store,
    sir_load,
    sir_br,
    sir_br_cond,
    sir_type_convert
};

std::string quoted_name(const std::string&);

struct value_t {
public:
    enum class kind {
        null,
        variable,
        literal
    };

public:
    kind value_kind;
    std::string content;

public:
    static auto null() {
        return value_t {
            .value_kind = value_t::kind::null,
            .content = ""
        };
    }
    static auto variable(const std::string& name) {
        return value_t {
            .value_kind = value_t::kind::variable,
            .content = name
        };
    }
    static auto literal(const std::string& value) {
        return value_t {
            .value_kind = value_t::kind::literal,
            .content = value
        };
    }

public:
    friend std::ostream& operator<<(std::ostream& out, const value_t& value) {
        switch(value.value_kind) {
            case value_t::kind::variable: out << "%" << value.content; break;
            case value_t::kind::literal: out << value.content; break;
            default: break;
        }
        return out;
    }
};

class sir {
private:
    sir_kind type;

public:
    sir(sir_kind k): type(k) {}
    virtual ~sir() = default;
    virtual void dump(std::ostream&) const = 0;
    auto get_ir_type() const { return type; }

public:
    template<typename T>
    auto to() {
        return reinterpret_cast<T*>(this);
    }
};

class sir_nop: public sir {
private:
    std::string info;

public:
    sir_nop(const std::string& i): sir(sir_kind::sir_nop), info(i) {}
    ~sir_nop() override = default;
    void dump(std::ostream& os) const override {
        if (info.size()) {
            os << "; " << info;
        }
        os << "\n";
    }
};

class sir_alloca: public sir {
private:
    std::string variable;
    std::string type;

public:
    sir_alloca(const std::string& v, const std::string& t):
        sir(sir_kind::sir_alloca), variable(v), type(t) {}
    ~sir_alloca() override = default;
    void dump(std::ostream&) const override;
    const auto& get_variable_name() const { return variable; }
    const auto& get_type_name() const { return type; }
};

class sir_temp_ptr: public sir {
/*
used to get temporary pointer of numbering variable
    %_1.real = alloca i32
this operand will do this:
    %1 = getelementptr i32, i32* %_1.real, i32 0
and %1 is i32*
*/
private:
    std::string target;
    std::string source;
    std::string type;

public:
    sir_temp_ptr(const std::string& tgt,
                 const std::string& src,
                 const std::string type):
        sir(sir_kind::sir_temp_ptr), target(tgt),
        source(src), type(type) {}
    ~sir_temp_ptr() override = default;
    void dump(std::ostream&) const override;
    const auto& get_type() const { return type; }
    void set_source(const std::string& src) { source = src; }
};

class sir_block: public sir {
private:
    std::vector<sir_alloca*> allocas;
    std::vector<sir_alloca*> move_register;
    std::vector<sir*> stmts;

public:
    sir_block(): sir(sir_kind::sir_block) {}
    ~sir_block() override;
    void dump(std::ostream&) const override;

    void add_alloca(sir_alloca* node) { allocas.push_back(node); }
    void add_move_register(sir_alloca* node) { move_register.push_back(node); }
    void add_stmt(sir* node) { stmts.push_back(node); }
    void add_nop(const std::string& info = "") {
        stmts.push_back(new sir_nop(info));
    }
    auto stmt_size() const { return stmts.size(); }
    bool back_is_ret_stmt() const {
        return stmts.size() && stmts.back()->get_ir_type()==sir_kind::sir_ret;
    }

    const auto& get_stmts() const { return stmts; }
    auto& get_mut_stmts() { return stmts; }
    const auto& get_move_register() const { return move_register; }
    auto& get_mut_move_register() { return move_register; }
};

class sir_ret: public sir {
private:
    std::string type;
    value_t value;

public:
    sir_ret(const std::string& t, const value_t& v):
        sir(sir_kind::sir_ret), type(t), value(v) {}
    ~sir_ret() override = default;
    void dump(std::ostream&) const override;
};

class sir_string: public sir {
private:
    usize index;
    usize length;
    value_t target;

public:
    sir_string(const usize sl, const usize i, const value_t& tgt):
        sir(sir_kind::sir_str), index(i), length(sl), target(tgt) {}
    ~sir_string() override = default;
    void dump(std::ostream&) const override;
};

class sir_zeroinitializer: public sir {
private:
    value_t destination;
    std::string type;

public:
    sir_zeroinitializer(const value_t& dst, const std::string& t):
        sir(sir_kind::sir_zeroinitializer), destination(dst), type(t) {}
    ~sir_zeroinitializer() override = default;
    void dump(std::ostream&) const override;
};

class sir_call_index: public sir {
private:
    value_t source;
    value_t destination;
    value_t index;
    std::string type;
    std::string index_type;

public:
    sir_call_index(const value_t& src,
                   const value_t& dst,
                   const value_t& idx,
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
    value_t destination;
    std::vector<std::string> args_type;
    std::vector<value_t> args;

public:
    sir_call_func(const std::string& n,
                  const std::string& rt,
                  const value_t& dst):
        sir(sir_kind::sir_call_func), name(n),
        return_type(rt), destination(dst) {}
    ~sir_call_func() override = default;
    void add_arg_type(const std::string& t) { args_type.push_back(t); }
    void add_arg(const value_t& a) { args.push_back(a); }
    void dump(std::ostream&) const override;
};

class sir_neg: public sir {
private:
    value_t source;
    value_t destination;
    bool is_integer;
    std::string type;

public:
    sir_neg(const value_t& src,
            const value_t& dst,
            bool is_int,
            const std::string& t):
        sir(sir_kind::sir_neg),
        source(src), destination(dst),
        is_integer(is_int), type(t) {}
    ~sir_neg() override = default;
    void dump(std::ostream&) const override;
};

class sir_bnot: public sir {
private:
    value_t source;
    value_t destination;
    std::string type;

public:
    sir_bnot(const value_t& src,
             const value_t& dst,
             const std::string& t):
        sir(sir_kind::sir_bnot),
        source(src), destination(dst),
        type(t) {}
    ~sir_bnot() override = default;
    void dump(std::ostream&) const override;
};

class sir_lnot: public sir {
private:
    value_t source;
    value_t destination;
    std::string type;

public:
    sir_lnot(const value_t& src,
             const value_t& dst,
             const std::string& t):
        sir(sir_kind::sir_lnot),
        source(src), destination(dst),
        type(t) {}
    ~sir_lnot() override = default;
    void dump(std::ostream&) const override;
};

class sir_add: public sir {
private:
    value_t left;
    value_t right;
    value_t destination;
    bool is_integer;
    std::string type;

public:
    sir_add(const value_t& l,
            const value_t& r,
            const value_t& dst,
            bool is_int,
            const std::string& t):
        sir(sir_kind::sir_add),
        left(l), right(r), destination(dst), is_integer(is_int), type(t) {}
    ~sir_add() override = default;
    void dump(std::ostream&) const override;
};

class sir_sub: public sir {
private:
    value_t left;
    value_t right;
    value_t destination;
    bool is_integer;
    std::string type;

public:
    sir_sub(const value_t& l,
            const value_t& r,
            const value_t& dst,
            bool is_int,
            const std::string& t):
        sir(sir_kind::sir_sub),
        left(l), right(r), destination(dst), is_integer(is_int), type(t) {}
    ~sir_sub() override = default;
    void dump(std::ostream&) const override;
};

class sir_mul: public sir {
private:
    value_t left;
    value_t right;
    value_t destination;
    bool is_integer;
    std::string type;

public:
    sir_mul(const value_t& l,
            const value_t& r,
            const value_t& dst,
            bool is_int,
            const std::string& t):
        sir(sir_kind::sir_mul),
        left(l), right(r), destination(dst), is_integer(is_int), type(t) {}
    ~sir_mul() override = default;
    void dump(std::ostream&) const override;
};

class sir_div: public sir {
private:
    value_t left;
    value_t right;
    value_t destination;
    bool is_integer;
    bool is_signed;
    std::string type;

public:
    sir_div(const value_t& l,
            const value_t& r,
            const value_t& dst,
            bool is_int,
            bool is_sign,
            const std::string& t):
        sir(sir_kind::sir_mul),
        left(l), right(r), destination(dst), is_integer(is_int),
        is_signed(is_sign), type(t) {}
    ~sir_div() override = default;
    void dump(std::ostream&) const override;
};

class sir_rem: public sir {
private:
    value_t left;
    value_t right;
    value_t destination;
    bool is_integer;
    bool is_signed;
    std::string type;

public:
    sir_rem(const value_t& l,
            const value_t& r,
            const value_t& dst,
            bool is_int,
            bool is_sign,
            const std::string& t):
        sir(sir_kind::sir_rem),
        left(l), right(r), destination(dst), is_integer(is_int),
        is_signed(is_sign), type(t) {}
    ~sir_rem() override = default;
    void dump(std::ostream&) const override;
};

class sir_band: public sir {
private:
    value_t left;
    value_t right;
    value_t destination;
    std::string type;

public:
    sir_band(const value_t& l,
             const value_t& r,
             const value_t& dst,
             const std::string& t):
        sir(sir_kind::sir_band),
        left(l), right(r), destination(dst), type(t) {}
    ~sir_band() override = default;
    void dump(std::ostream&) const override;
};

class sir_bxor: public sir {
private:
    value_t left;
    value_t right;
    value_t destination;
    std::string type;

public:
    sir_bxor(const value_t& l,
             const value_t& r,
             const value_t& dst,
             const std::string& t):
        sir(sir_kind::sir_bxor),
        left(l), right(r), destination(dst), type(t) {}
    ~sir_bxor() override = default;
    void dump(std::ostream&) const override;
};

class sir_bor: public sir {
private:
    value_t left;
    value_t right;
    value_t destination;
    std::string type;

public:
    sir_bor(const value_t& l,
            const value_t& r,
            const value_t& dst,
            const std::string& t):
        sir(sir_kind::sir_bor),
        left(l), right(r), destination(dst), type(t) {}
    ~sir_bor() override = default;
    void dump(std::ostream&) const override;
};

class sir_cmp: public sir {
public:
    enum class kind {
        cmp_eq,
        cmp_neq,
        cmp_ge,
        cmp_gt,
        cmp_le,
        cmp_lt
    };

private:
    kind cmp_type;
    value_t left;
    value_t right;
    value_t destination;
    bool is_integer;
    bool is_signed;
    std::string type;

public:
    sir_cmp(kind ct,
            const value_t& l,
            const value_t& r,
            const value_t& dst,
            bool is_int,
            bool is_sign,
            const std::string& t):
        sir(sir_kind::sir_cmp), cmp_type(ct),
        left(l), right(r), destination(dst),
        is_integer(is_int), is_signed(is_sign), type(t) {}
    ~sir_cmp() override = default;
    void dump(std::ostream&) const override;
};

class sir_label: public sir {
private:
    usize label_count;
    std::string comment;

public:
    sir_label(usize count, const std::string& cm = ""):
        sir(sir_kind::sir_label), label_count(count), comment(cm) {}
    ~sir_label() override = default;
    void dump(std::ostream&) const override;
    std::string get_label() const;
};

class sir_place_holder_label: public sir {
private:
    usize label_count;

public:
    sir_place_holder_label(usize count):
        sir(sir_kind::sir_place_holder_label), label_count(count) {}
    ~sir_place_holder_label() override = default;
    void dump(std::ostream&) const override;
    std::string get_label() const;
};

class sir_store: public sir {
private:
    std::string type;
    value_t source;
    value_t destination;

public:
    sir_store(const std::string& t,
              const value_t& src,
              const value_t& dst):
        sir(sir_kind::sir_store), type(t), source(src), destination(dst) {}
    ~sir_store() override = default;
    void dump(std::ostream&) const override;
};

class sir_load: public sir {
private:
    std::string type;
    value_t source;
    value_t destination;

public:
    sir_load(const std::string& t,
             const value_t& src,
             const value_t& dst):
        sir(sir_kind::sir_load), type(t), source(src), destination(dst) {}
    ~sir_load() override = default;
    void dump(std::ostream&) const override;
};

class sir_br: public sir {
private:
    usize label;

public:
    sir_br(usize dst): sir(sir_kind::sir_br), label(dst) {}
    ~sir_br() override = default;
    void dump(std::ostream&) const override;

    void set_label(usize dst) { label = dst; }
    std::string get_label() const;
};

class sir_br_cond: public sir {
private:
    std::string condition;
    usize label_true;
    usize label_false;

public:
    sir_br_cond(const std::string& cd, u64 dst_true, u64 dst_false):
        sir(sir_kind::sir_br_cond), condition(cd),
        label_true(dst_true), label_false(dst_false) {}
    ~sir_br_cond() override = default;
    void dump(std::ostream&) const override;
    std::string get_label_true() const;
    std::string get_label_false() const;

public:
    void set_true_label(u64 dst_true) {
        label_true = dst_true;
    }
    void set_false_label(u64 dst_false) {
        label_false = dst_false;
    }
};

class sir_type_convert: public sir {
private:
    value_t source;
    value_t destination;
    std::string src_type;
    std::string dst_type;

private:
    std::string convert_instruction(char, int, char, int) const;

public:
    sir_type_convert(const value_t& src,
                     const value_t& dst,
                     const std::string& st,
                     const std::string& dt):
        sir(sir_kind::sir_type_convert),
        source(src), destination(dst),
        src_type(st), dst_type(dt) {}
    ~sir_type_convert() override = default;
    void dump(std::ostream&) const override;
};

}