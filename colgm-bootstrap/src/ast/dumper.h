#include "ast/visitor.h"

#include <vector>
#include <iostream>
#include <cstring>
#include <sstream>

namespace colgm {

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
    void pop_indent() {indent.pop_back();}
    void set_last() {indent.back() = "+--";}
    void dump_indent() {
        if (indent.size() && indent.back()=="|  ") {
            indent.back() = "|--";
        }
        for(const auto& i : indent) {
            std::cout << i;
        }
    }
    std::string format_location(const span& location) {
        std::stringstream ss;
        ss << " -> " << location << "\n";
        return ss.str();
    }

public:
    dumper(): indent({}) {}
    bool visit_root(root*) override;
    bool visit_identifier(identifier*) override;
    bool visit_number_literal(number_literal*) override;
    bool visit_string_literal(string_literal*) override;
    bool visit_type_def(type_def*) override;
    bool visit_struct_field(struct_field*) override;
    bool visit_struct_decl(struct_decl*) override;
    bool visit_param(param*) override;
    bool visit_param_list(param_list*) override;
    bool visit_func_decl(func_decl*) override;
    bool visit_impl_struct(impl_struct*) override;
    bool visit_binary_operator(binary_operator*) override;
    bool visit_call_index(call_index*) override;
    bool visit_call_func_args(call_func_args*) override;
    bool visit_call_field(call_field*) override;
    bool visit_ptr_call_field(ptr_call_field*) override;
    bool visit_call_path(call_path*) override;
    bool visit_call(call*) override;
    bool visit_assignment(assignment*) override;
    bool visit_definition(definition*) override;
    bool visit_cond_stmt(cond_stmt*) override;
    bool visit_if_stmt(if_stmt*) override;
    bool visit_while_stmt(while_stmt*) override;
    bool visit_in_stmt_expr(in_stmt_expr*) override;
    bool visit_ret_stmt(ret_stmt*) override;
    bool visit_code_block(code_block*) override;
};

}