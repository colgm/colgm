#pragma once

#include "report.h"

#include <vector>

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
private:
    kind mir_kind;
    span location;

public:
    mir(kind k, const span& loc): mir_kind(k), location(loc) {}
    virtual ~mir() = default;
};

class mir_nop: public mir {
public:
    mir_nop(const span& loc): mir(kind::mir_nop, loc) {}
    ~mir_nop() override = default;
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

public:
    void add_content(mir* n) { content.push_back(n); }
};

}