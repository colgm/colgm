#include "ast/delete_disabled_node.h"
#include "colgm.h"

#include <vector>

namespace colgm::ast {

bool delete_disabled_node::check_enable_if(cond_compile* cc) {
    if (cc->get_condition_name() != "enable_if") {
        return false;
    }
    for(const auto& i : cc->get_comments()) {
        if (i.first != "target_os" && i.first != "arch") {
            return false;
        }
    }

    if (cc->get_comments().count("target_os") &&
        cc->get_comments().count("arch")) {
        const auto& target_os = cc->get_comments().at("target_os");
        const auto& arch = cc->get_comments().at("arch");
        return target_os == get_platform() && arch == get_arch();
    }

    if (cc->get_comments().count("target_os")) {
        const auto& target_os = cc->get_comments().at("target_os");
        return target_os == get_platform();
    }

    if (cc->get_comments().count("arch")) {
        const auto& arch = cc->get_comments().at("arch");
        return arch == get_arch();
    }

    return false;
}

void delete_disabled_node::scan(root* node) {
    std::vector<decl*> new_root_decls;
    for(auto i : node->get_decls()) {
        if (i->get_ast_type() != ast_type::ast_cond_compile) {
            new_root_decls.push_back(i);
            continue;
        }
        auto cc = static_cast<cond_compile*>(i);
        // should be enabled
        if (check_enable_if(cc)) {
            new_root_decls.push_back(cc->get_enabled_decl());
            cc->set_enabled_decl(nullptr);
            delete cc;
        } else {
            delete cc;
        }
    }
    node->reset_decls(new_root_decls);
}

}