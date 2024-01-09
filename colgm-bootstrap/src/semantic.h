#pragma once
#include "ast/ast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "code/ir.h"
#include "ast/visitor.h"

#include <unordered_map>
#include <cstring>
#include <sstream>
#include <vector>

namespace colgm {

enum class symbol_kind {
    basic_kind,
    struct_kind,
    func_kind
};

struct symbol {
    std::string name;
    std::string type;
    uint64_t type_pointer_level;

    std::string type_to_string() {
        auto result = type;
        for(uint64_t i = 0; i<type_pointer_level; ++i) {
            result += "*";
        }
        return result;
    }
};

std::ostream& operator<<(std::ostream&, const symbol&);

struct colgm_func {
    std::string name;
    symbol return_type;
    std::vector<symbol> parameters;
};

struct colgm_struct {
    std::string name;
    std::vector<symbol> field;
    std::unordered_map<std::string, colgm_func> method;
};

struct semantic_context {
    std::unordered_map<std::string, colgm_struct> structs;
    std::unordered_map<std::string, colgm_func> functions;
    std::unordered_map<std::string, symbol_kind> symbols;
};

class semantic {
private:
    error err;
    semantic_context ctx;

private:
    void analyse_single_struct(struct_decl*);
    void analyse_structs(root*);
    colgm_func analyse_single_func(func_decl*);
    void analyse_functions(root*);
    void analyse_single_impl(impl_struct*);
    void analyse_impls(root*);

private:
    void resolve_func(func_decl*);
    void resolve_impl(impl_struct*);
    void resolve_function_block(root*);

public:
    const error& analyse(root*);
    void dump();
    const auto& get_context() const { return ctx; }
};

}