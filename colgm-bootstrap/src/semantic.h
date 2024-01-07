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

class semantic: public visitor {
private:
    error err;
    std::unordered_map<std::string, colgm_struct> structs;
    std::unordered_map<std::string, colgm_func> functions;
    std::unordered_map<std::string, symbol_kind> symbols;
    std::unordered_map<std::string, std::string> basic_type_convert_mapper = {
        {"i64", "i64"},
        {"i32", "i32"},
        {"i16", "i16"},
        {"i8", "i8"},
        {"u64", "i64"},
        {"u32", "i32"},
        {"u16", "i16"},
        {"u8", "i8"},
        {"f64", "double"},
        {"f32", "float"},
        {"void", "void"},
        {"bool", "i1"}
    };

private:
    std::vector<ir_struct*> struct_decls;
    std::vector<ir*> generated_codes;
    std::string impl_struct_name = "";
    ir_code_block* cb = nullptr;
    uint64_t jmp_label_count = 0;

    void emit(ir* i) { generated_codes.push_back(i); }
    void emit_struct_decl(ir_struct* s) { struct_decls.push_back(s); }
    std::string generate_type_string(type_def*);
    bool visit_struct_decl(struct_decl*) override;
    bool visit_func_decl(func_decl*) override;
    bool visit_impl_struct(impl_struct*) override;
    bool visit_number_literal(number_literal*) override;
    bool visit_string_literal(string_literal*) override;
    bool visit_call(call*) override;
    bool visit_call_index(call_index*) override;
    bool visit_call_field(call_field*) override;
    bool visit_call_func_args(call_func_args*) override;
    bool visit_binary_operator(binary_operator*) override;
    bool visit_ret_stmt(ret_stmt*) override;
    bool visit_definition(definition*) override;
    bool visit_assignment(assignment*) override;
    bool visit_while_stmt(while_stmt*) override;
    bool visit_if_stmt(if_stmt*) override;

private:
    void analyse_single_struct(struct_decl*);
    void analyse_structs(root*);
    colgm_func analyse_single_func(func_decl*);
    void analyse_functions(root*);
    void analyse_single_impl(impl_struct*);
    void analyse_impls(root*);

public:
    ~semantic() {
        for(auto i : struct_decls) {
            delete i;
        }
        for(auto i : generated_codes) {
            delete i;
        }
    }
    const error& analyse(root*);
    void dump();
    void dump_code(std::ostream&);
    const auto& get_structs() const { return structs; }
    const auto& get_functions() const { return functions; }
    const auto& get_symbols() const { return symbols; }
};

}