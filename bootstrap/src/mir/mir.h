#pragma once

#include "sema/symbol.h"
#include "report.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>

namespace colgm::mir {

enum class kind {
    mir_nop,
    mir_block,
    mir_unary,
    mir_binary,
    mir_type_convert,
    mir_nil,
    mir_number,
    mir_string,
    mir_char,
    mir_bool,
    mir_call_id,
    mir_call_index,
    mir_call_func,
    mir_call_field,
    mir_call_path,
    mir_ptr_call_field,
    mir_define,
    mir_assign,
};

class mir {
protected:
    kind mir_kind;
    span location;

public:
    mir(kind k, const span& loc): mir_kind(k), location(loc) {}
    virtual ~mir() = default;
    virtual void dump(const std::string&, std::ostream&) {}
};

class mir_nop: public mir {
public:
    mir_nop(const span& loc): mir(kind::mir_nop, loc) {}
    ~mir_nop() override = default;
    void dump(const std::string&, std::ostream&) override;
};

class mir_block: public mir {
private:
    std::vector<mir*> content;

public:
    mir_block(const span& loc): mir(kind::mir_block, loc) {}
    ~mir_block() override {
        for(auto i : content) {
            delete i;
        }
    }
    void dump(const std::string&, std::ostream&) override;

public:
    void add_content(mir* n) { content.push_back(n); }
};

class mir_nil: public mir {
private:
    type resolve_type;

public:
    mir_nil(const span& loc, const type& t):
        mir(kind::mir_nil, loc), resolve_type(t) {}
    ~mir_nil() override = default;
    void dump(const std::string&, std::ostream&) override;
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
};

}