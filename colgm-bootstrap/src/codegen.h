#include "ast/visitor.h"

#include <fstream>

namespace colgm {

class codegen: public visitor {
private:
    std::ofstream out;

public:
    codegen(const std::string& file): out(file) {}
    bool visit_identifier(identifier*) override;
    bool visit_type_def(type_def*) override;
    bool visit_param(param*) override;
    bool visit_param_list(param_list*) override;
    bool visit_func_decl(func_decl*) override;
};

}