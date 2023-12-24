#include "ast/visitor.h"

namespace colgm {

bool visitor::visit_ast_node(ast_node* node) {
    node->accept(this);
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

bool visitor::visit_stmt(stmt* node) {
    node->accept(this);
    return true;
}

}