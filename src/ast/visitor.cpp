#include "ast/visitor.h"

namespace colgm {

bool visitor::visit_node(node* node) {
    node->accept(this);
    return true;
}

bool visitor::visit_root(root* node) {
    for(auto i : node->get_use_statements()) {
        i->accept(this);
    }
    for(auto i : node->get_declarations()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_decl(decl* node) {
    node->accept(this);
    return true;
}

bool visitor::visit_expr(expr* node) {
    node->accept(this);
    return true;
}

bool visitor::visit_identifier(identifier* node) {
    return true;
}

bool visitor::visit_stmt(stmt* node) {
    node->accept(this);
    return true;
}

bool visitor::visit_use_stmt(use_stmt* node) {
    for(auto i : node->get_path()) {
        i->accept(this);
    }
    switch(node->get_use_kind()) {
        case use_stmt::use_kind::use_all: break;
        case use_stmt::use_kind::use_single:
            node->get_single_use()->accept(this); break;
        case use_stmt::use_kind::use_specify:
            for(auto i : node->get_specified_use()) {
                i->accept(this);
            }
            break;
    }
    return true;
}

}