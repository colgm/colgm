#pragma once

#include "report.h"
#include "ast/visitor.h"

#include <vector>

namespace colgm::ast {

struct scope {
    std::vector<std::vector<defer_stmt*>> scopes;

    void new_scope() {
        scopes.push_back({});
    }
    void pop_scope() {
        if (scopes.empty()) {
            return;
        }
        scopes.pop_back();
    }
    bool empty() const {
        for (const auto& i : scopes) {
            if (!i.empty()) {
                return false;
            }
        }
        return true;
    }
};

class replace_defer: public visitor {
private:
    error& err;
    bool in_defer_block;
    std::vector<scope> top_scopes;

private:
    void add_top_scope() {
        top_scopes.push_back({});
    }
    void add_sub_scope() {
        top_scopes.back().new_scope();
    }
    void pop_top_scope() {
        if (top_scopes.empty()) {
            return;
        }
        top_scopes.pop_back();
    }
    void pop_sub_scope() {
        top_scopes.back().pop_scope();
    }
    void add_defer_stmt(defer_stmt* n) {
        top_scopes.back().scopes.back().push_back(n);
    }

    void insert_defer(defer_stmt*, std::vector<stmt*>&);
    void insert_defer_on_return(std::vector<stmt*>&);
    void insert_defer_on_block_exit(std::vector<stmt*>&);

private:
    bool visit_func_decl(func_decl*) override;
    bool visit_while_stmt(while_stmt*) override;
    bool visit_for_stmt(for_stmt*) override;
    bool visit_forindex(forindex*) override;
    bool visit_foreach(foreach*) override;
    bool visit_defer_stmt(defer_stmt*) override;
    bool visit_ret_stmt(ret_stmt*) override;
    bool visit_continue_stmt(continue_stmt*) override;
    bool visit_break_stmt(break_stmt*) override;
    bool visit_code_block(code_block*) override;

public:
    replace_defer(error& e):
        err(e), in_defer_block(false) {}
    void scan(root*);
};

}