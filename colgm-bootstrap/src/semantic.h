#pragma once
#include "ast/ast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"

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
};

std::ostream& operator<<(std::ostream&, const symbol&);

struct colgm_struct {
    std::string name;
    std::vector<symbol> field;
};

struct colgm_func {
    std::string name;
    symbol return_type;
    std::vector<symbol> parameters;
};

class semantic {
private:
    error err;
    std::unordered_map<std::string, colgm_struct> structs;
    std::unordered_map<std::string, colgm_func> functions;
    std::unordered_map<std::string, symbol_kind> symbols;

private:
    void analyse_single_struct(struct_decl*);
    void analyse_structs(root*);
    void analyse_single_func(func_decl*);
    void analyse_functions(root*);

public:
    const error& analyse(root*);
    void dump();
};

}