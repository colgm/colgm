#pragma once

#include "err.h"
#include "sema/symbol.h"

#include <vector>

namespace colgm {

enum class ast_type {
    ast_null = 0,
    ast_root,
    ast_binary_operator,
    ast_identifier,
    ast_number_literal,
    ast_string_literal,
    ast_bool_literal,
    ast_call_index,
    ast_call_func_args,
    ast_call_field,
    ast_ptr_call_field,
    ast_call_path,
    ast_call,
    ast_assignment,
    ast_type_def,
    ast_struct_field,
    ast_struct_decl,
    ast_param,
    ast_param_list,
    ast_func_decl,
    ast_impl,
    ast_definition,
    ast_cond_stmt,
    ast_if_stmt,
    ast_while_stmt,
    ast_in_stmt_expr,
    ast_ret_stmt,
    ast_code_block
};

class visitor;

class decl;
class type_def;
class struct_field;
class struct_decl;
class param;
class param_list;
class func_decl;
class impl_struct;
class expr;
class null;
class identifier;
class number_literal;
class string_literal;
class bool_literal;
class call_index;
class call_func_args;
class call_field;
class ptr_call_field;
class call_path;
class call;
class assignment;
class stmt;
class definition;
class cond_stmt;
class if_stmt;
class while_stmt;
class in_stmt_expr;
class ret_stmt;
class code_block;

class node {
private:
    ast_type node_type;
    span location;
    type resolve;

public:
    node(ast_type type, const span& loc):
        node_type(type), location(loc), resolve({"", 0}) {}
    virtual ~node() = default;
    virtual void accept(visitor*) = 0;

    const auto& get_location() const { return location; }
    const auto get_ast_type() const { return node_type; }
    const auto& get_resolve() const { return resolve; }
    void set_resolve_type(const type& rs) { resolve = rs; }
    void update_location(const span& end) {
        location.end_line = end.end_line;
        location.end_column = end.end_column;
    }
};

class root: public node {
private:
    std::vector<decl*> declarations;

public:
    root(const span& loc): node(ast_type::ast_root, loc) {}
    ~root() override;
    void accept(visitor*) override;

    void add_decl(decl* node) { declarations.push_back(node); }
    const auto& get_decls() const { return declarations; }
};

}