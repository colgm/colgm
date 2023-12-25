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
    bool visit_use_stmt(use_stmt*) override;
    bool visit_specified_use(specified_use*) override;
};

}