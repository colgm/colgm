#include "ast/replace_defer.h"

namespace colgm::ast {

void replace_defer::insert_defer(defer_stmt* ds, std::vector<stmt*>& v) {
    for (auto i : ds->get_block()->get_stmts()) {
        v.push_back(i->clone());
    }
}

void replace_defer::insert_defer_on_return(std::vector<stmt*>& v) {
    for (const auto& this_top_scope : top_scopes) {
        for (const auto& sub_scope : this_top_scope.scopes) {
            for (auto i : sub_scope) {
                insert_defer(i, v);
            }
        }
    }
}

bool replace_defer::should_insert_on_return() const {
    for (const auto& this_top_scope : top_scopes) {
        for (const auto& sub_scope : this_top_scope.scopes) {
            for (auto i : sub_scope) {
                if (!i->get_block()->get_stmts().empty()) {
                    return true;
                }
            }
        }
    }
    return false;
}

void replace_defer::insert_defer_on_block_exit(std::vector<stmt*>& v) {
    for (const auto& sub_scope : top_scopes.back().scopes) {
        for (auto i : sub_scope) {
            insert_defer(i, v);
        }
    }
}

void replace_defer::insert_defer_on_subscope_exit(std::vector<stmt*>& v) {
    if (top_scopes.empty() || top_scopes.back().empty()) {
        return;
    }
    for (auto i : top_scopes.back().scopes.back()) {
        insert_defer(i, v);
    }
}

bool replace_defer::visit_func_decl(func_decl* d) {
    return_type = d->get_return_type();
    if (d->get_code_block()) {
        top_scopes.clear();
        add_top_scope();
        d->get_code_block()->accept(this);
        top_scopes.clear();
    }
    return_type = nullptr;
    return true;
}

bool replace_defer::visit_while_stmt(while_stmt* node) {
    add_top_scope();
    node->get_block()->accept(this);
    pop_top_scope();
    return true;
}

bool replace_defer::visit_for_stmt(for_stmt* node) {
    add_top_scope();
    node->get_block()->accept(this);
    pop_top_scope();
    return true;
}

bool replace_defer::visit_forindex(forindex* node) {
    add_top_scope();
    node->get_body()->accept(this);
    pop_top_scope();
    return true;
}

bool replace_defer::visit_foreach(foreach* node) {
    add_top_scope();
    node->get_body()->accept(this);
    pop_top_scope();
    return true;
}

bool replace_defer::visit_defer_stmt(defer_stmt* s) {
    if (in_defer_block) {
        err.err(s->get_location(), "defer statement cannot be nested");
        return false;
    }

    in_defer_block = true;
    add_top_scope();
    s->get_block()->accept(this);
    pop_top_scope();
    in_defer_block = false;

    return true;
}

bool replace_defer::visit_ret_stmt(ret_stmt* s) {
    if (in_defer_block) {
        err.err(s->get_location(),
            "return statement cannot be nested in defer block"
        );
    }
    return true;
}

bool replace_defer::visit_continue_stmt(continue_stmt* s) {
    if (in_defer_block) {
        err.err(s->get_location(),
            "continue statement cannot be nested in defer block"
        );
    }
    return true;
}

bool replace_defer::visit_break_stmt(break_stmt* s) {
    if (in_defer_block) {
        err.err(s->get_location(),
            "break statement cannot be nested in defer block"
        );
    }
    return true;
}

bool replace_defer::visit_code_block(code_block* b) {
    add_sub_scope();

    // collect defers and generate new block
    std::vector<stmt*> new_stmts;
    bool has_defer_in_this_sub_scope = false;
    for (auto i : b->get_stmts()) {
        i->accept(this);
        if (i->is(ast_type::ast_defer_stmt)) {
            has_defer_in_this_sub_scope = true;
            add_defer_stmt(reinterpret_cast<defer_stmt*>(i));
            new_stmts.push_back(i);
            continue;
        }
        if (i->is(ast_type::ast_continue_stmt) ||
            i->is(ast_type::ast_break_stmt)) {
            insert_defer_on_block_exit(new_stmts);
            new_stmts.push_back(i);
        } else if (i->is(ast_type::ast_ret_stmt)) {
            // replace return value with a temporary variable
            // and put the value before defer insertion place
            // to avoid UAF issues
            //
            //   defer statement;
            //   ...
            //   return value;
            // -->
            //   var defer.tmp.0 = value;
            //   statement;
            //   return defer.tmp.0;
            //
            auto ret_node = reinterpret_cast<ret_stmt*>(i);
            if (ret_node->get_value() && should_insert_on_return()) {
                auto val_node = ret_node->get_value();
                definition* def_node = new definition(
                    ret_node->get_location(),
                    "defer.tmp." + std::to_string(tmp_return_value_id)
                );
                def_node->set_init_value(val_node);
                if (return_type) {
                    def_node->set_type(return_type->clone());
                }

                call* call_node = new call(val_node->get_location());
                call_id* call_id_node = new call_id(val_node->get_location());
                call_id_node->set_id(new identifier(
                    val_node->get_location(),
                    "defer.tmp." + std::to_string(tmp_return_value_id)
                ));
                call_node->set_head(call_id_node);
                ret_node->set_value(call_node);

                tmp_return_value_id++;
                new_stmts.push_back(def_node);
            }
            insert_defer_on_return(new_stmts);
            new_stmts.push_back(i);
        } else {
            new_stmts.push_back(i);
        }
    }
    // insert defer when top scope exits
    if (in_top_scope()) {
        if (b->back_not_block_exit()) {
            insert_defer_on_block_exit(new_stmts);
        }
    } else if (has_defer_in_this_sub_scope) {
        if (b->back_not_block_exit()) {
            insert_defer_on_subscope_exit(new_stmts);
        }
    }
    b->reset_stmt_with(new_stmts);

    pop_sub_scope();

    // delete defer now
    new_stmts.clear();
    for (auto i : b->get_stmts()) {
        if (i->is(ast_type::ast_defer_stmt)) {
            delete i;
        } else {
            new_stmts.push_back(i);
        }
    }
    b->reset_stmt_with(new_stmts);
    return true;
}

void replace_defer::scan(root* r) {
    r->accept(this);
}

}
