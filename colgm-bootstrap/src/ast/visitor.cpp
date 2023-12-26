#include "ast/visitor.h"

namespace colgm {

bool visitor::visit_node(node* node) {
    node->accept(this);
    return true;
}

bool visitor::visit_root(root* node) {
    for(auto i : node->get_decls()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_decl(decl* node) {
    node->accept(this);
    return true;
}

bool visitor::visit_type_def(type_def* node) {
    node->get_name()->accept(this);
    return true;
}

bool visitor::visit_param(param* node) {
    node->get_name()->accept(this);
    node->get_type()->accept(this);
    return true;
}

bool visitor::visit_param_list(param_list* node) {
    for(auto i : node->get_params()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_func_decl(func_decl* node) {
    node->get_params()->accept(this);
    node->get_return_type()->accept(this);
    node->get_code_block()->accept(this);
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

bool visitor::visit_code_block(code_block* node) {
    // TODO
    return true;
}

}