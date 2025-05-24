#include "ast/visitor.h"

#include <vector>
#include <iostream>
#include <cstring>
#include <sstream>

namespace colgm::ast {

class dumper: public visitor {
private:
    std::vector<std::string> indent;

private:
    void push_indent() {
        if (indent.size()) {
            if (indent.back()=="|--") {
                indent.back() = "|  ";
            } else if (indent.back()=="+--") {
                indent.back() = "   ";
            }
        }
        indent.push_back("|--");
    }
    void pop_indent() { indent.pop_back(); }
    void set_last() { indent.back() = "+--"; }
    void dump_indent() {
        if (indent.size() && indent.back()=="|  ") {
            indent.back() = "|--";
        }
        for(const auto& i : indent) {
            std::cout << i;
        }
    }
    std::string format_location(node* n) {
        std::stringstream ss;
        ss << " -> " << n->get_location();
        if (n->is_redirected() &&
            n->get_redirect_location() != n->get_location().file) {
            ss << " <redirect " << n->get_redirect_location() << ">";
        }
        ss << "\n";
        return ss.str();
    }

private:
    bool visit_root(root*) override;
    bool visit_identifier(identifier*) override;
    bool visit_nil_literal(nil_literal*) override;
    bool visit_number_literal(number_literal*) override;
    bool visit_string_literal(string_literal*) override;
    bool visit_char_literal(char_literal*) override;
    bool visit_bool_literal(bool_literal*) override;
    bool visit_array_list(array_list*) override;
    bool visit_cond_compile(cond_compile*) override;
    bool visit_type_def(type_def*) override;
    bool visit_generic_type_list(generic_type_list*) override;
    bool visit_enum_decl(enum_decl*) override;
    bool visit_field_pair(field_pair*) override;
    bool visit_struct_decl(struct_decl*) override;
    bool visit_tagged_union_decl(tagged_union_decl*) override;
    bool visit_param(param*) override;
    bool visit_param_list(param_list*) override;
    bool visit_func_decl(func_decl*) override;
    bool visit_impl_struct(impl_struct*) override;
    bool visit_unary_operator(unary_operator*) override;
    bool visit_binary_operator(binary_operator*) override;
    bool visit_type_convert(type_convert*) override;
    bool visit_call_id(call_id*) override;
    bool visit_call_index(call_index*) override;
    bool visit_call_func_args(call_func_args*) override;
    bool visit_get_field(get_field*) override;
    bool visit_ptr_get_field(ptr_get_field*) override;
    bool visit_init_pair(init_pair*) override;
    bool visit_initializer(initializer*) override;
    bool visit_call_path(call_path*) override;
    bool visit_call(call*) override;
    bool visit_assignment(assignment*) override;
    bool visit_use_stmt(use_stmt*) override;
    bool visit_definition(definition*) override;
    bool visit_cond_stmt(cond_stmt*) override;
    bool visit_if_stmt(if_stmt*) override;
    bool visit_match_case(match_case*) override;
    bool visit_match_stmt(match_stmt*) override;
    bool visit_while_stmt(while_stmt*) override;
    bool visit_for_stmt(for_stmt*) override;
    bool visit_forindex(forindex*) override;
    bool visit_foreach(foreach*) override;
    bool visit_in_stmt_expr(in_stmt_expr*) override;
    bool visit_ret_stmt(ret_stmt*) override;
    bool visit_continue_stmt(continue_stmt*) override;
    bool visit_break_stmt(break_stmt*) override;
    bool visit_code_block(code_block*) override;

public:
    dumper(): indent({}) {}
    static void dump(node* n) {
        dumper d;
        n->accept(&d);
    }
};

}