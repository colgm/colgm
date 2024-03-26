#include "sema/context.h"

namespace colgm {

bool semantic_context::find_symbol(const std::string& name) {
    for(auto i = scope.rbegin(); i!=scope.rend(); ++i) {
        if (i->count(name)) {
            return true;
        }
    }
    return false;
}

type semantic_context::get_symbol(const std::string& name) {
    for(auto i = scope.rbegin(); i!=scope.rend(); ++i) {
        if (i->count(name)) {
            return i->at(name);
        }
    }
    return type::error_type();
}

void semantic_context::add_symbol(const std::string& name, const type& t) {
    scope.back().insert({name, t});
}

void semantic_context::dump_structs() const {
    for(const auto& domain: global.domain) {
        std::cout << "info of " << domain.first << ":\n";
        for(const auto& i : domain.second.structs) {
            std::cout << "  struct " << i.first << " {\n";
            for(const auto& field : i.second.ordered_field) {
                std::cout << "    " << field.name << ": " << field.symbol_type;
                if (field.name!=i.second.ordered_field.back().name) {
                    std::cout << ",";
                }
                std::cout << "\n";
            }
            std::cout << "  }\n";
            if (i.second.method.empty()) {
                continue;
            }
            std::cout << "  impl " << i.first << " {\n";
            for(const auto& method : i.second.method) {
                dump_single_function(method.second, "  ");
            }
            std::cout << "  }\n";
        }
    }
}

void semantic_context::dump_functions() const {
    for(const auto& domain: global.domain) {
        std::cout << "info of " << domain.first << ":\n";
        for(const auto& func : domain.second.functions) {
            dump_single_function(func.second, "");
        }
    }
}

void semantic_context::dump_single_function(const colgm_func& func,
                                            const std::string& indent) const {
    std::cout << indent << "  func " << func.name << "(";
    for(const auto& param : func.parameters) {
        std::cout << param.name << ": " << param.symbol_type;
        if (param.name!=func.parameters.back().name) {
            std::cout << ", ";
        }
    }
    std::cout << ") -> " << func.return_type << "\n";
}

}