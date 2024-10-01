#include "sema/context.h"

namespace colgm {

bool sema_context::find_symbol(const std::string& name) {
    for(auto i = scope.rbegin(); i!=scope.rend(); ++i) {
        if (i->count(name)) {
            return true;
        }
    }
    return false;
}

type sema_context::get_symbol(const std::string& name) {
    for(auto i = scope.rbegin(); i!=scope.rend(); ++i) {
        if (i->count(name)) {
            return i->at(name);
        }
    }
    return type::error_type();
}

void sema_context::add_symbol(const std::string& name, const type& t) {
    scope.back().insert({name, t});
}

void sema_context::dump_generics(const std::vector<std::string>& v) const {
    if (v.empty()) {
        return;
    }
    std::cout << "<";
    for(const auto& i : v) {
        std::cout << i;
        if (i != v.back()) {
            std::cout << ", ";
        }
    }
    std::cout << ">";
}

void sema_context::dump_structs() const {
    for(const auto& domain : global.domain) {
        if (domain.second.structs.empty()) {
            continue;
        }
        std::cout << "info of " << domain.first << ":\n";
        for(const auto& i : domain.second.structs) {
            std::cout << "  struct " << i.first;
            dump_generics(i.second.generic_template);
            std::cout << " {\n";
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

void sema_context::dump_enums() const {
    for(const auto& domain : global.domain) {
        if (domain.second.enums.empty()) {
            continue;
        }
        std::cout << "info of " << domain.first << ":\n";
        for(const auto& en : domain.second.enums) {
            std::cout << "  enum " << en.second.name << " {\n";
            for(const auto& i : en.second.ordered_member) {
                std::cout << "    " << i << ": ";
                std::cout << en.second.members.at(i) << "\n";
            }
            std::cout << "  }\n";
        }
    }
}

void sema_context::dump_functions() const {
    for(const auto& domain : global.domain) {
        if (domain.second.functions.empty()) {
            continue;
        }
        std::cout << "info of " << domain.first << ":\n";
        for(const auto& func : domain.second.functions) {
            dump_single_function(func.second, "");
        }
    }
}

void sema_context::dump_single_function(const colgm_func& func,
                                            const std::string& indent) const {
    std::cout << indent << "  func " << func.name;
    dump_generics(func.generic_template);
    std::cout << "(";
    for(const auto& param : func.parameters) {
        std::cout << param.name << ": " << param.symbol_type;
        if (param.name!=func.parameters.back().name) {
            std::cout << ", ";
        }
    }
    std::cout << ") -> " << func.return_type << "\n";
}

}