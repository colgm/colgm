#pragma once

#include "ast/visitor.h"
#include "mir/mir.h"
#include "report.h"

#include <cstring>
#include <sstream>
#include <unordered_map>

namespace colgm::mir {

class ast2mir: public visitor {
private:
    error err;

private:
    std::unordered_map<std::string, mir_block*> impls;

public:
    ~ast2mir() {
        for(const auto& i : impls) {
            delete i.second;
        }
    }

public:
    auto& generate(root* ast_root) {
        ast_root->accept(this);
        return err;
    }
    auto& get_mutable_ir() const { return impls; }
};

}