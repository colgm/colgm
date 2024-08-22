#pragma once

#include "sema/symbol.h"
#include "package/package.h"

#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <sstream>

namespace colgm {

struct global_symbol_table {
    std::unordered_set<std::string> constant_string;
    std::unordered_map<std::string, colgm_module> domain;
};

struct symbol_info {
    sym_kind kind;
    std::string loc_file;
};

struct semantic_context {
    static inline global_symbol_table global;

    // store the file name this context belongs to
    std::string this_file;

    // store global symbol used in this file
    std::unordered_map<std::string, symbol_info> global_symbol;

    // local scope
    std::vector<std::unordered_map<std::string, type>> scope = {};

    bool find_symbol(const std::string&);
    void add_symbol(const std::string&, const type&);
    type get_symbol(const std::string&);
    void push_new_level() { scope.push_back({}); }
    void pop_new_level() { scope.pop_back(); }

    void dump_structs() const;
    void dump_enums() const;
    void dump_functions() const;
    void dump_single_function(const colgm_func&,
                              const std::string& indent = "") const;

public:
    sym_kind search_symbol_kind(const type& t) const {
        if (!global.domain.count(t.loc_file)) {
            return sym_kind::error_kind;
        }
        const auto& domain = global.domain.at(t.loc_file);
        if (domain.enums.count(t.name)) {
            return sym_kind::enum_kind;
        }
        if (domain.structs.count(t.name)) {
            return sym_kind::struct_kind;
        }
        if (domain.functions.count(t.name)) {
            return sym_kind::func_kind;
        }
        return sym_kind::error_kind;
    }
};

}