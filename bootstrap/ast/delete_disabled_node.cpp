#include "ast/delete_disabled_node.h"
#include "report.h"

#include <vector>
#include <unordered_set>

namespace colgm::ast {

bool delete_disabled_node::check_enable_if(error& err, cond_compile* cc) {
    if (cc->get_condition_name() != "enable_if") {
        return false;
    }

    const std::unordered_set<std::string> valid_conditions = {
        "target_os", "arch"
    };
    bool contain_invalid = false;
    for(const auto& i : cc->get_conds()) {
        if (!valid_conditions.count(i.first)) {
            contain_invalid = true;
            err.err(cc->get_location(), "invalid enable_if condition");
        }
    }
    if (contain_invalid) {
        return false;
    }

    if (cc->get_conds().count("target_os") &&
        cc->get_conds().count("arch")) {
        const auto& target_os = cc->get_conds().at("target_os");
        const auto& target_arch = cc->get_conds().at("arch");
        return target_os == platform && target_arch == arch;
    }

    if (cc->get_conds().count("target_os")) {
        const auto& target_os = cc->get_conds().at("target_os");
        return target_os == platform;
    }

    if (cc->get_conds().count("arch")) {
        const auto& target_arch = cc->get_conds().at("arch");
        return target_arch == arch;
    }

    return false;
}

bool delete_disabled_node::check_conds(error& err,
                                       const std::vector<cond_compile*>& conds) {
    for (auto i : conds) {
        if (i->get_condition_name() != "enable_if") {
            err.err(
                i->get_location(),
                "this compile condition is not supported."
            );
            continue;
        }
        return check_enable_if(err, i);
    }
    return true;
}

void delete_disabled_node::report_not_supported_condition(error& err,
                                                          impl_struct* node) {
    static const std::unordered_set<std::string> valid_conditions = {
        "is_trivial", "is_non_trivial"
    };
    for (auto i : node->get_methods()) {
        for (auto j : i->get_conds()) {
            if (valid_conditions.count(j->get_condition_name())) {
                continue;
            }
            err.err(
                j->get_location(),
                "this compile condition is not supported."
            );
        }
    }
}

void delete_disabled_node::scan(error& err, root* node) {
    std::vector<decl*> new_root_decls;
    for(auto i : node->get_decls()) {
        switch (i->get_ast_type()) {
            case ast_type::ast_enum_decl:
                if (check_conds(err, static_cast<enum_decl*>(i)->get_conds())) {
                    new_root_decls.push_back(i);
                } else {
                    delete i;
                }
                break;
            case ast_type::ast_struct_decl:
                if (check_conds(err, static_cast<struct_decl*>(i)->get_conds())) {
                    new_root_decls.push_back(i);
                } else {
                    delete i;
                }
                break;
            case ast_type::ast_func_decl:
                if (check_conds(err, static_cast<func_decl*>(i)->get_conds())) {
                    new_root_decls.push_back(i);
                } else {
                    delete i;
                }
                break;
            case ast_type::ast_impl:
                if (check_conds(err, static_cast<impl_struct*>(i)->get_conds())) {
                    new_root_decls.push_back(i);
                    report_not_supported_condition(err, static_cast<impl_struct*>(i));
                } else {
                    delete i;
                }
                break;
            default:
                new_root_decls.push_back(i);
                break;
        }
    }
    node->reset_decls(new_root_decls);
}

}