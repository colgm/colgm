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
    bool visit_type_def(type_def*) override;
    bool visit_struct_field(struct_field*) override;
    bool visit_struct_decl(struct_decl*) override;
    bool visit_param(param*) override;
    bool visit_param_list(param_list*) override;
    bool visit_func_decl(func_decl*) override;
    bool visit_code_block(code_block*) override;
};

}