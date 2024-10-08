#pragma once

#include "sema/symbol.h"
#include "package/package.h"

#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <sstream>
#include <vector>

namespace colgm {

struct global_symbol_table {
    std::unordered_set<std::string> constant_string;
    std::unordered_map<std::string, colgm_module> domain;
};

struct symbol_info {
    sym_kind kind;
    std::string loc_file;
    bool is_public = false;
};

struct sema_context {
    static inline global_symbol_table global;

    // store the file name this context belongs to
    std::string this_file;

    // store global symbols used in current scope
    // both normal and generic symbols are stored
    std::unordered_map<std::string, symbol_info> global_symbol;

    // only store generics in current scope
    std::unordered_set<std::string> generic_symbol;

    // store generics type name
    // for field analysis or generic methods type infer
    std::unordered_set<std::string> generics;

    // local scope
    std::vector<std::unordered_map<std::string, type>> local_scope = {};

    bool find_local(const std::string&);
    void add_local(const std::string&, const type&);
    type get_local(const std::string&);
    void push_level() { local_scope.push_back({}); }
    void pop_level() { local_scope.pop_back(); }

public:
    void insert(const std::string& name,
                const symbol_info& si,
                bool is_generic = false) {
        global_symbol.insert({name, si});
        if (is_generic) {
            generic_symbol.insert(name);
        }
    }

public:
    void dump_generics(const std::vector<std::string>&) const;
    void dump_structs() const;
    void dump_single_struct(const colgm_struct&) const;
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