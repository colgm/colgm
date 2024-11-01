#pragma once

#include "sema/primitive.h"
#include "sema/symbol.h"
#include "package/package.h"

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <sstream>
#include <vector>

namespace colgm {

struct global_symbol_table {
    // store all string literals
    std::unordered_set<std::string> constant_string;

    // store primitive types
    std::unordered_map<std::string, colgm_primitive> primitives;

    // store all modules
    std::unordered_map<std::string, colgm_module> domain;
};

struct sema_context {
    static inline global_symbol_table global;

    // store the file name this context belongs to
    std::string this_file;

    // temporarily store generics type name,
    // for field analysis or generic methods type infer.
    // may clear after type infer.
    std::unordered_set<std::string> generics;

    // temporary local scope, may clear after block analysis.
    std::vector<std::unordered_map<std::string, type>> local_scope = {};

public:
    bool find_local(const std::string&);
    void add_local(const std::string&, const type&);
    type get_local(const std::string&);
    void push_level() { local_scope.push_back({}); }
    void pop_level() { local_scope.pop_back(); }

public:
    auto& global_symbol() {
        return global.domain.at(this_file).global_symbol;
    }
    auto& generic_symbol() {
        return global.domain.at(this_file).generic_symbol;
    }
    const auto& global_symbol() const {
        return global.domain.at(this_file).global_symbol;
    }
    const auto& generic_symbol() const {
        return global.domain.at(this_file).generic_symbol;
    }
    auto& get_domain(const std::string& file) {
        assert(global.domain.count(file) && "unknown domain");
        return global.domain.at(file);
    }
    const auto& get_domain(const std::string& file) const {
        assert(global.domain.count(file) && "unknown domain");
        return global.domain.at(file);
    }

public:
    void insert(const std::string& name,
                const symbol_info& si,
                bool is_generic = false) {
        global_symbol().insert({name, si});
        if (is_generic) {
            generic_symbol().insert({name, si});
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
        if (global.primitives.count(t.name_for_search())) {
            return sym_kind::basic_kind;
        }
        if (!global.domain.count(t.loc_file)) {
            return sym_kind::error_kind;
        }
        const auto& domain = global.domain.at(t.loc_file);
        if (domain.enums.count(t.name_for_search())) {
            return sym_kind::enum_kind;
        }
        if (domain.structs.count(t.name_for_search())) {
            return sym_kind::struct_kind;
        }
        if (domain.functions.count(t.name_for_search())) {
            return sym_kind::func_kind;
        }
        return sym_kind::error_kind;
    }
};

}