#pragma once

#include "colgm.h"
#include "sir/debug_info.h"

#include <cstdint>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace colgm {

enum class sir_kind {
    sir_null = 0,
    sir_block,
    sir_alloca,
    sir_temp_ptr,
    sir_ret,
    sir_str,
    sir_zeroinitializer,
    sir_get_index,
    sir_get_field,
    sir_call,
    sir_neg,
    sir_bnot,
    sir_lnot,
    sir_add,
    sir_fadd,
    sir_sub,
    sir_mul,
    sir_div,
    sir_rem,
    sir_band,
    sir_bxor,
    sir_bor,
    sir_cmp,
    sir_basic_block,
    sir_store,
    sir_load,
    sir_br,
    sir_br_cond,
    sir_switch,
    sir_type_convert,
    sir_array_cast
};

class sir_basic_block;

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
    static auto null(const std::string& name = "") {
        return value_t {
            .value_kind = value_t::kind::null,
            .content = name
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
            case value_t::kind::null:
                // if content is not empty, means there's an error in codegen
                if (!value.content.empty()) {
                    out << "<null: " << value.content << ">";
                }
                break;
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

class sir_alloca: public sir {
public:
    struct array_type_info {
        std::string array_base_type;
        usize size;
    };

private:
    std::string variable;
    std::string type;
    array_type_info array_info;

public:
    sir_alloca(const std::string& v, const std::string& t):
        sir(sir_kind::sir_alloca), variable(v), type(t),
        array_info({"", 0}) {}
    // this constructor is used to allocate array on stack
    sir_alloca(const std::string& v, const array_type_info& ati):
        sir(sir_kind::sir_alloca), variable(v), type(""),
        array_info(ati) {}
    ~sir_alloca() override = default;
    void dump(std::ostream&) const override;
    void set_array(const array_type_info& ati) { array_info = ati; }
    const auto& get_variable_name() const { return variable; }
    const auto& get_type_name() const { return type; }
};

class sir_temp_ptr: public sir {
/*
used to get temporary pointer of numbering variable
    %_1.r = alloca i32
this operand will do this:
    %1 = getelementptr i32, i32* %_1.r, i32 0
and %1 is i32*
*/
private:
    std::string target;
    std::string source;
    std::string type;
    std::string comment; // for tail comment

public:
    sir_temp_ptr(const std::string& tgt,
                 const std::string& src,
                 const std::string& type,
                 const std::string& cmt = ""):
        sir(sir_kind::sir_temp_ptr), target(tgt),
        source(src), type(type), comment(cmt) {}
    ~sir_temp_ptr() override = default;
    void dump(std::ostream&) const override;
    const auto& get_type() const { return type; }
    void set_source(const std::string& src) { source = src; }
};

class sir_block: public sir {
private:
    std::vector<sir_basic_block*> basic_blocks;

public:
    sir_block(): sir(sir_kind::sir_block) {}
    ~sir_block() override;
    void dump(std::ostream&) const override;
    void add_basic_block(sir_basic_block* node) { basic_blocks.push_back(node); }
    const auto& get_basic_blocks() const { return basic_blocks; }
    auto& get_mutable_basic_blocks() { return basic_blocks; }
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
    value_t target;
    std::string type;

public:
    sir_zeroinitializer(const value_t& tgt, const std::string& t):
        sir(sir_kind::sir_zeroinitializer), target(tgt), type(t) {}
    ~sir_zeroinitializer() override = default;
    void dump(std::ostream&) const override;
};

class sir_get_index: public sir {
private:
    value_t source;
    value_t destination;
    value_t index;
    std::string type;
    std::string index_type;

public:
    sir_get_index(const value_t& src,
                  const value_t& dst,
                  const value_t& idx,
                  const std::string& t,
                  const std::string& it):
        sir(sir_kind::sir_get_index), source(src),
        destination(dst), index(idx), type(t), index_type(it) {}
    ~sir_get_index() override = default;
    void dump(std::ostream&) const override;
};

class sir_get_field: public sir {
private:
    value_t destination;
    value_t source;
    std::string struct_name;
    usize index;

public:
    sir_get_field(const value_t& dst,
                  const value_t& src,
                  const std::string& sn,
                  usize i):
        sir(sir_kind::sir_get_field),
        destination(dst), source(src),
        struct_name(sn), index(i) {}
    ~sir_get_field() override = default;
    void dump(std::ostream&) const override;
};

class sir_call: public sir {
private:
    std::string name;
    std::string return_type;
    value_t destination;
    std::vector<std::string> args_type;
    std::vector<value_t> args;
    bool with_va_args;
    u64 with_va_args_real_param_size;
    u64 debug_info_index;

public:
    sir_call(const std::string& n,
             const std::string& rt,
             const value_t& dst):
        sir(sir_kind::sir_call), name(n),
        return_type(rt), destination(dst),
        with_va_args(false), with_va_args_real_param_size(0),
        debug_info_index(DI_node::DI_ERROR_INDEX) {}
    ~sir_call() override = default;
    const auto& get_name() const { return name; }
    const auto& get_destination() const { return destination; }
    const auto& get_return_type() const { return return_type; }
    const auto& get_args_type() const { return args_type; }
    const auto& get_args() const { return args; }
    void add_arg_type(const std::string& t) { args_type.push_back(t); }
    void add_arg(const value_t& a) { args.push_back(a); }
    void set_with_va_args(bool b) { with_va_args = b; }
    void set_with_va_args_real_param_size(u64 s) {
        with_va_args_real_param_size = s;
    }
    void set_debug_info_index(u64 i) { debug_info_index = i; }
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
    std::string type;

public:
    sir_add(const value_t& l,
            const value_t& r,
            const value_t& dst,
            const std::string& t):
        sir(sir_kind::sir_add),
        left(l), right(r), destination(dst), type(t) {}
    ~sir_add() override = default;
    void dump(std::ostream&) const override;
};

class sir_fadd: public sir {
private:
    value_t left;
    value_t right;
    value_t destination;
    std::string type;

public:
    sir_fadd(const value_t& l,
             const value_t& r,
             const value_t& dst,
             const std::string& t):
        sir(sir_kind::sir_fadd),
        left(l), right(r), destination(dst), type(t) {}
    ~sir_fadd() override = default;
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

class sir_basic_block: public sir {
private:
    usize label_count;
    std::vector<sir*> stmts;
    std::vector<sir_basic_block*> preds;
    std::vector<sir_basic_block*> succs;
    std::string comment;

public:
    sir_basic_block(usize count, const std::string& cm = ""):
        sir(sir_kind::sir_basic_block), label_count(count), comment(cm) {}
    ~sir_basic_block() override;
    void add_stmt(sir* node) { stmts.push_back(node); }
    bool back_is_ret_stmt() const {
        return stmts.size() && stmts.back()->get_ir_type() == sir_kind::sir_ret;
    }

    auto& get_preds() { return preds; }
    auto& get_succs() { return succs; }

    const auto& get_stmts() const { return stmts; }
    auto& get_mut_stmts() { return stmts; }
    std::string get_label() const;
    auto get_label_num() const { return label_count; }
    void dump(std::ostream&) const override;
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
    const auto& get_source() const { return source; }
    const auto& get_destination() const { return destination; }
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
    auto get_label_num() const { return label; }
};

class sir_br_cond: public sir {
private:
    value_t condition;
    usize label_true;
    usize label_false;

public:
    sir_br_cond(const value_t& cd, u64 dst_true, u64 dst_false):
        sir(sir_kind::sir_br_cond), condition(cd),
        label_true(dst_true), label_false(dst_false) {}
    ~sir_br_cond() override = default;
    void dump(std::ostream&) const override;
    std::string get_label_true() const;
    std::string get_label_false() const;
    auto get_label_true_num() const { return label_true; }
    auto get_label_false_num() const { return label_false; }

public:
    void set_true_label(u64 dst_true) {
        label_true = dst_true;
    }
    void set_false_label(u64 dst_false) {
        label_false = dst_false;
    }
};

class sir_switch: public sir {
private:
    value_t source;
    usize label_default;
    std::vector<std::pair<i64, usize>> label_cases;

public:
    sir_switch(const value_t& src):
        sir(sir_kind::sir_switch), source(src) {}
    ~sir_switch() override = default;
    void dump(std::ostream&) const override;

public:
    void add_case(i64 value, usize label) {
        label_cases.push_back({value, label});
    }
    void set_default_label(usize dst_default) {
        label_default = dst_default;
    }
    auto get_default_label_num() const { return label_default; }
    std::string get_default_label() const;
    const auto& get_cases() const { return label_cases; }
    std::vector<std::string> get_case_labels() const;
};

class sir_type_convert: public sir {
private:
    value_t source;
    value_t destination;
    std::string src_type;
    std::string dst_type;
    bool src_unsigned;
    bool dst_unsigned;

private:
    std::string convert_instruction(char, int, char, int) const;

public:
    sir_type_convert(const value_t& src,
                     const value_t& dst,
                     const std::string& st,
                     const std::string& dt,
                     bool su,
                     bool du):
        sir(sir_kind::sir_type_convert),
        source(src), destination(dst),
        src_type(st), dst_type(dt),
        src_unsigned(su), dst_unsigned(du) {}
    ~sir_type_convert() override = default;
    void dump(std::ostream&) const override;
};

class sir_array_cast: public sir {
private:
    value_t source;
    value_t destination;
    std::string type;
    usize array_size;

public:
    sir_array_cast(const value_t& src,
                   const value_t& dst,
                   const std::string& t,
                   usize size):
        sir(sir_kind::sir_array_cast), source(src),
        destination(dst), type(t), array_size(size) {}
    ~sir_array_cast() override = default;
    void dump(std::ostream&) const override;
};

}