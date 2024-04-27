#pragma once 

#include "colgm.h"
#include "code/hir.h"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <sstream>

namespace colgm {

struct ir_context {
    std::vector<hir_struct> struct_decls;
    std::vector<hir_func*> func_decls;
    std::vector<hir_func*> func_impls;
    std::unordered_map<std::string, u64> const_strings;
    std::unordered_map<std::string, std::unordered_set<std::string>> used_basic_convert_method;

private:
    bool passes_already_executed = false;

private:
    void dump_raw_string(std::ostream&, const std::string&) const;
    void dump_const_string(std::ostream&) const;

private:
    std::string convert_instruction(char, int, char, int) const;
    void dump_used_basic_convert_method(std::ostream&) const;

private:
    void dump_struct_size_method(std::ostream&) const;
    void dump_struct_alloc_method(std::ostream&) const;
    void dump_struct_delete_method(std::ostream&) const;

private:
    bool check_used(const std::string&) const;
    void check_and_dump_malloc(std::ostream&) const;
    void check_and_dump_free(std::ostream&) const;

public:
    ~ir_context() {
        for(auto i : func_decls) {
            delete i;
        }
        for(auto i : func_impls) {
            delete i;
        }
    }
    void dump_code(std::ostream&);
};

}