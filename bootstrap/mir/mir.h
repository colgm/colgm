#pragma once

#include "sema/type.h"
#include "report.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>

namespace colgm::mir {

enum class kind {
    mir_block = 1,
    mir_unary,
    mir_binary,
    mir_type_convert,
    mir_nil,
    mir_number,
    mir_string,
    mir_char,
    mir_bool,
    mir_array,
    mir_struct_init,
    mir_call,
    mir_call_id,
    mir_call_index,
    mir_call_func,
    mir_get_field,
    mir_get_path,
    mir_ptr_get_field,
    mir_define,
    mir_assign,
    mir_if,
    mir_branch,
    mir_switch_case,
    mir_switch,
    mir_break,
    mir_continue,
    mir_loop,
    mir_return
};

class visitor;

class mir {
protected:
    kind mir_kind;
    span location;

public:
    mir(kind k, const span& loc): mir_kind(k), location(loc) {}
    virtual ~mir() = default;
    virtual void dump(const std::string&, std::ostream&) {}
    virtual void accept(visitor*);

public:
    auto get_kind() const { return mir_kind; }
    const auto& get_location() const { return location; }
};

class mir_block: public mir {
private:
    std::vector<mir*> content;

public:
    mir_block(const span& loc): mir(kind::mir_block, loc) {}
    ~mir_block() override {
        for (auto i : content) {
            delete i;
        }
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    void add_content(mir* n) { content.push_back(n); }
    const auto& get_content() const { return content; }
    auto& get_mutable_content() { return content; }
};

class mir_unary: public mir {
public:
    enum class opr_kind {
        neg,
        bnot,
        lnot
    };

private:
    opr_kind opr;
    type resolve_type;
    mir_block* value;

public:
    mir_unary(const span& loc, opr_kind o, const type& t, mir_block* v):
        mir(kind::mir_unary, loc), opr(o), resolve_type(t), value(v) {}
    ~mir_unary() override {
        delete value;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_value() const { return value; }
    auto get_opr() const { return opr; }
    const auto& get_type() const { return resolve_type; }
};

class mir_binary: public mir {
public:
    enum class opr_kind {
        add,
        sub,
        mult,
        div,
        rem,
        cmpeq,
        cmpneq,
        less,
        leq,
        grt,
        geq,
        cmpand,
        cmpor,
        band,
        bor,
        bxor,
    };

private:
    opr_kind opr;
    type resolve_type;
    mir_block* left;
    mir_block* right;

public:
    mir_binary(const span& loc,
               opr_kind o,
               const type& t,
               mir_block* l,
               mir_block* r):
        mir(kind::mir_binary, loc), opr(o), resolve_type(t),
        left(l), right(r) {}
    ~mir_binary() override {
        delete left;
        delete right;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_left() const { return left; }
    auto get_right() const { return right; }
    auto get_opr() const { return opr; }
    const auto& get_type() const { return resolve_type; }
};

class mir_type_convert: public mir {
private:
    mir_block* source;
    type target_type;

public:
    mir_type_convert(const span& loc, mir_block* src, const type& tt):
        mir(kind::mir_type_convert, loc), source(src), target_type(tt) {}
    ~mir_type_convert() override {
        delete source;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    const auto& get_target_type() const { return target_type; }
    auto get_source() const { return source; }
};

class mir_nil: public mir {
private:
    type resolve_type;

public:
    mir_nil(const span& loc, const type& t):
        mir(kind::mir_nil, loc), resolve_type(t) {}
    ~mir_nil() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    const auto& get_type() const { return resolve_type; }
};

class mir_number: public mir {
private:
    std::string literal;
    type resolve_type;

public:
    mir_number(const span& loc, const std::string& l, const type& t):
        mir(kind::mir_number, loc), literal(l), resolve_type(t) {}
    ~mir_number() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    const auto& get_literal() const { return literal; }
    const auto& get_type() const { return resolve_type; }
};

class mir_string: public mir {
private:
    std::string literal;
    type resolve_type;

public:
    mir_string(const span& loc, const std::string& l, const type& t):
        mir(kind::mir_string, loc), literal(l), resolve_type(t) {}
    ~mir_string() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    const auto& get_literal() const { return literal; }
    const auto& get_type() const { return resolve_type; }
};

class mir_char: public mir {
private:
    char literal;
    type resolve_type;

public:
    mir_char(const span& loc, const char l, const type& t):
        mir(kind::mir_char, loc), literal(l), resolve_type(t) {}
    ~mir_char() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_literal() const { return literal; }
    const auto& get_type() const { return resolve_type; }
};

class mir_bool: public mir {
private:
    bool literal;
    type resolve_type;

public:
    mir_bool(const span& loc, const bool l, const type& t):
        mir(kind::mir_bool, loc), literal(l), resolve_type(t) {}
    ~mir_bool() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_literal() const { return literal; }
    const auto& get_type() const { return resolve_type; }
};

class mir_array: public mir {
private:
    u64 size;
    type resolve_type;
    mir_block* value;

public:
    mir_array(const span& loc, u64 sz, const type& t):
        mir(kind::mir_array, loc), size(sz), resolve_type(t),
        value(nullptr) {}
    ~mir_array() override;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_size() const { return size; }
    const auto& get_type() const { return resolve_type; }
    void set_value(mir_block* v) { value = v; }
    auto get_value() const { return value; }
};

class mir_struct_init: public mir {
public:
    struct field {
        std::string name;
        mir_block* content;
        type resolve_type;
    };

private:
    std::vector<field> fields;
    type resolve_type;

public:
    mir_struct_init(const span& loc, const type& t):
        mir(kind::mir_struct_init, loc), resolve_type(t) {}
    ~mir_struct_init() override;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    void add_field(const std::string& n, mir_block* c, const type& t) {
        fields.push_back({n, c, t});
    }
    const auto& get_fields() const { return fields; }
    const auto& get_type() const { return resolve_type; }

};

class mir_call: public mir {
private:
    mir_block* content;
    type resolve_type;

public:
    mir_call(const span& loc, const type& t, mir_block* c):
        mir(kind::mir_call, loc), resolve_type(t), content(c) {}
    ~mir_call() override {
        delete content;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_content() const { return content; }
    const auto& get_type() const { return resolve_type; }
};

class mir_call_id: public mir {
private:
    std::string name;
    type resolve_type;

public:
    mir_call_id(const span& loc, const std::string& n, const type& t):
        mir(kind::mir_call_id, loc), name(n), resolve_type(t) {}
    ~mir_call_id() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    const auto& get_name() const { return name; }
    const auto& get_type() const { return resolve_type; }
};

class mir_call_index: public mir {
private:
    type resolve_type;
    mir_block* index;

public:
    mir_call_index(const span& loc, const type& t, mir_block* i):
        mir(kind::mir_call_index, loc), resolve_type(t), index(i) {}
    ~mir_call_index() override {
        delete index;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_index() const { return index; }
    const auto& get_type() const { return resolve_type; }
};

class mir_call_func: public mir {
private:
    mir_block* args;
    type resolve_type;

public:
    mir_call_func(const span& loc, mir_block* a, const type& t):
        mir(kind::mir_call_func, loc), args(a), resolve_type(t) {}
    ~mir_call_func() override { delete args; }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_args() const { return args; }
    const auto& get_type() const { return resolve_type; }
};

class mir_get_field: public mir {
private:
    std::string name;
    type resolve_type;

public:
    mir_get_field(const span& loc, const std::string& n, const type& t):
        mir(kind::mir_get_field, loc), name(n), resolve_type(t) {}
    ~mir_get_field() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    const auto& get_name() const { return name; }
    const auto& get_type() const { return resolve_type; }
};

class mir_ptr_get_field: public mir {
private:
    std::string name;
    type resolve_type;

public:
    mir_ptr_get_field(const span& loc, const std::string& n, const type& t):
        mir(kind::mir_ptr_get_field, loc), name(n), resolve_type(t) {}
    ~mir_ptr_get_field() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    const auto& get_name() const { return name; }
    const auto& get_type() const { return resolve_type; }
};

class mir_get_path: public mir {
private:
    std::string name;
    type resolve_type;

public:
    mir_get_path(const span& loc, const std::string& n, const type& t):
        mir(kind::mir_get_path, loc), name(n), resolve_type(t) {}
    ~mir_get_path() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    const auto& get_name() const { return name; }
    const auto& get_type() const { return resolve_type; }
};

class mir_define: public mir {
private:
    std::string name;
    mir_block* init_value;
    type resolve_type;

public:
    mir_define(const span& loc,
               const std::string& n,
               mir_block* iv,
               const type& rt):
        mir(kind::mir_define, loc), name(n),
        init_value(iv), resolve_type(rt) {}
    ~mir_define() override {
        delete init_value;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    const auto& get_name() const { return name; }
    const auto& get_type() const { return resolve_type; }
    auto get_init_value() const { return init_value; }
};

class mir_assign: public mir {
public:
    enum class opr_kind {
        eq,
        addeq,
        subeq,
        multeq,
        diveq,
        remeq,
        andeq,
        xoreq,
        oreq
    };

private:
    opr_kind opr;
    mir_block* left;
    mir_block* right;

public:
    mir_assign(const span& loc, const opr_kind o, mir_block* l, mir_block* r):
        mir(kind::mir_assign, loc), opr(o), left(l), right(r) {}
    ~mir_assign() override {
        delete left;
        delete right;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_left() const { return left; }
    auto get_right() const { return right; }
    auto get_opr() const { return opr; }
};

class mir_if: public mir {
private:
    mir_block* condition;
    mir_block* content;

public:
    mir_if(const span& loc, mir_block* cond, mir_block* ct):
        mir(kind::mir_if, loc), condition(cond), content(ct) {}
    ~mir_if() override {
        delete condition;
        delete content;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_condition() const { return condition; }
    auto get_content() const { return content; }
};

class mir_branch: public mir {
private:
    std::vector<mir_if*> branch;

public:
    mir_branch(const span& loc):
        mir(kind::mir_branch, loc) {}
    ~mir_branch() override {
        for (auto i : branch) {
            delete i;
        }
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    void add(mir_if* b) { branch.push_back(b); }
    const auto& get_branch() const { return branch; }
};

class mir_switch_case: public mir {
private:
    i64 value;
    mir_block* content;

public:
    mir_switch_case(const span& loc, i64 v, mir_block* ct):
        mir(kind::mir_switch_case, loc), value(v), content(ct) {}
    ~mir_switch_case() override {
        delete content;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_value() const { return value; }
    auto get_content() const { return content; }
};

class mir_switch: public mir {
private:
    mir_block* condition;
    std::vector<mir_switch_case*> cases;
    mir_block* default_case;

public:
    mir_switch(const span& loc):
        mir(kind::mir_switch, loc), condition(nullptr), default_case(nullptr) {}
    ~mir_switch() override {
        delete condition;
        for (auto i : cases) {
            delete i;
        }
        delete default_case;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    void add(mir_switch_case* b) { cases.push_back(b); }
    void set_condition(mir_block* c) { condition = c; }
    void set_default_case(mir_block* c) { default_case = c; }
    auto get_condition() const { return condition; }
    const auto& get_cases() const { return cases; }
    auto get_default_case() const { return default_case; }
};

class mir_break: public mir {
public:
    mir_break(const span& loc): mir(kind::mir_break, loc) {}
    ~mir_break() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;
};

class mir_continue: public mir {
public:
    mir_continue(const span& loc): mir(kind::mir_continue, loc) {}
    ~mir_continue() override = default;
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;
};

class mir_loop: public mir {
private:
    mir_block* condition;
    mir_block* content;
    mir_block* update;

public:
    mir_loop(const span& loc, mir_block* cond, mir_block* ct, mir_block* ud):
        mir(kind::mir_loop, loc), condition(cond), content(ct), update(ud) {}
    ~mir_loop() override {
        delete condition;
        delete content;
        delete update;
    }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_condition() const { return condition; }
    auto get_content() const { return content; }
    auto get_update() const { return update; }
};

class mir_return: public mir {
private:
    mir_block* value;

public:
    mir_return(const span& loc, mir_block* v):
        mir(kind::mir_return, loc), value(v) {}
    ~mir_return() override { delete value; }
    void dump(const std::string&, std::ostream&) override;
    void accept(visitor*) override;

public:
    auto get_value() const { return value; }
};

}