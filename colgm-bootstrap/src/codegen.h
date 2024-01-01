#include "ast/visitor.h"
#include "semantic.h"

#include <fstream>
#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace colgm {

class codegen: public visitor {
private:
    std::ofstream out;
    std::vector<std::string> constant_strings;
    std::unordered_map<std::string, colgm_struct> structs;

public:
    codegen(const std::string& file): out(file) {}
    bool visit_root(root*) override;
    bool visit_identifier(identifier*) override;
    bool visit_string_literal(string_literal*) override;
    bool visit_type_def(type_def*) override;
    bool visit_struct_field(struct_field*) override;
    bool visit_struct_decl(struct_decl*) override;
    bool visit_param(param*) override;
    bool visit_param_list(param_list*) override;
    bool visit_func_decl(func_decl*) override;
};

}