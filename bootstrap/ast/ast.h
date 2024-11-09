#pragma once

#include "report.h"
#include "sema/symbol.h"

#include <vector>
#include <cassert>

namespace colgm::ast {

enum class ast_type {
    ast_null = 0,
    ast_root,
    ast_unary_operator,
    ast_binary_operator,
    ast_type_convert,
    ast_identifier,
    ast_nil_literal,
    ast_number_literal,
    ast_string_literal,
    ast_char_literal,
    ast_bool_literal,
    ast_array_literal,
    ast_call_id,
    ast_call_index,
    ast_call_func_args,
    ast_get_field,
    ast_ptr_get_field,
    ast_init_pair,
    ast_initializer,
    ast_call_path,
    ast_call,
    ast_assignment,
    ast_condition_comment,
    ast_type_def,
    ast_generic_type_list,
    ast_enum_decl,
    ast_struct_field,
    ast_struct_decl,
    ast_param,
    ast_param_list,
    ast_func_decl,
    ast_impl,
    ast_use_stmt,
    ast_definition,
    ast_cond_stmt,
    ast_if_stmt,
    ast_match_case,
    ast_match_stmt,
    ast_while_stmt,
    ast_for_stmt,
    ast_in_stmt_expr,
    ast_ret_stmt,
    ast_continue_stmt,
    ast_break_stmt,
    ast_code_block
};

class visitor;

class decl;
class type_def;
class generic_type_list;
class enum_decl;
class struct_field;
class struct_decl;
class param;
class param_list;
class func_decl;
class impl_struct;
class expr;
class null;
class unary_operator;
class binary_operator;
class type_convert;
class condition_comment;
class identifier;
class number_literal;
class string_literal;
class bool_literal;
class array_literal;
class call_id;
class call_index;
class call_func_args;
class get_field;
class ptr_get_field;
class init_pair;
class initializer;
class call_path;
class call;
class assignment;
class stmt;
class use_stmt;
class definition;
class cond_stmt;
class if_stmt;
class match_case;
class match_stmt;
class while_stmt;
class for_stmt;
class in_stmt_expr;
class ret_stmt;
class continue_stmt;
class break_stmt;
class code_block;

class node {
protected:
    ast_type node_type;
    span location; // original node location
    std::string redirect_location; // mostly used in copied generic nodes
    type resolve;

public:
    node(ast_type type, const span& loc):
        node_type(type), location(loc),
        redirect_location(""),
        resolve({"", "", 0}) {}
    virtual ~node() = default;
    virtual void accept(visitor*) = 0;
    virtual node* clone() const {
        assert(false && "not implemented: class node");
        return nullptr;
    };

public:
    const auto& get_location() const { return location; }
    const auto& get_redirect_location() const { return redirect_location; }
    const auto& get_file() const { return location.file; }
    const auto get_ast_type() const { return node_type; }
    const auto& get_resolve() const { return resolve; }

public:
    void set_resolve_type(const type& rs) { resolve = rs; }
    void set_redirect_location(const std::string& rl) { redirect_location = rl; }
    void update_location(const span& end) {
        location.end_line = end.end_line;
        location.end_column = end.end_column;
    }

public:
    bool is_redirected() const { return !redirect_location.empty(); }
};

class root: public node {
private:
    std::vector<use_stmt*> imports;
    std::vector<decl*> declarations;

public:
    root(const span& loc): node(ast_type::ast_root, loc) {}
    ~root() override;
    void accept(visitor*) override;
    root* clone() const override;

    void add_decl(decl* node) { declarations.push_back(node); }
    void add_use_stmt(use_stmt* node) { imports.push_back(node); }
    const auto& get_decls() const { return declarations; }
    const auto& get_use_stmts() const { return imports; }
};

}