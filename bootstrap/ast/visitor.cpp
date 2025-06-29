#include "ast/visitor.h"

namespace colgm::ast {

bool visitor::visit_node(node* node) {
    node->accept(this);
    return true;
}

bool visitor::visit_root(root* node) {
    for (auto i : node->get_use_stmts()) {
        i->accept(this);
    }
    for (auto i : node->get_decls()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_decl(decl* node) {
    node->accept(this);
    return true;
}

bool visitor::visit_cond_compile(cond_compile* node) {
    return true;
}

bool visitor::visit_type_def(type_def* node) {
    node->get_name()->accept(this);
    if (node->get_generic_types()) {
        node->get_generic_types()->accept(this);
    }
    if (node->get_array_length()) {
        node->get_array_length()->accept(this);
    }
    return true;
}

bool visitor::visit_generic_type_list(generic_type_list* node) {
    for (auto i : node->get_types()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_enum_decl(enum_decl* node) {
    node->get_name()->accept(this);
    for (const auto& i : node->get_member()) {
        i.name->accept(this);
        if (i.value) {
            i.value->accept(this);
        }
    }
    return true;
}

bool visitor::visit_field_pair(field_pair* node) {
    node->get_name()->accept(this);
    node->get_type()->accept(this);
    return true;
}

bool visitor::visit_struct_decl(struct_decl* node) {
    if (node->get_generic_types()) {
        node->get_generic_types()->accept(this);
    }
    for (auto i : node->get_fields()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_tagged_union_decl(tagged_union_decl* node) {
    for (auto i : node->get_members()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_param(param* node) {
    node->get_name()->accept(this);
    if (node->get_type()) {
        node->get_type()->accept(this);
    }
    return true;
}

bool visitor::visit_param_list(param_list* node) {
    for (auto i : node->get_params()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_func_decl(func_decl* node) {
    if (node->get_generic_types()) {
        node->get_generic_types()->accept(this);
    }
    node->get_params()->accept(this);
    node->get_return_type()->accept(this);
    if (node->get_code_block()) {
        node->get_code_block()->accept(this);
    }
    return true;
}

bool visitor::visit_impl(impl* node) {
    if (node->get_generic_types()) {
        node->get_generic_types()->accept(this);
    }
    for (auto i : node->get_methods()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_expr(expr* node) {
    node->accept(this);
    return true;
}

bool visitor::visit_unary_operator(unary_operator* node) {
    node->get_value()->accept(this);
    return true;
}

bool visitor::visit_binary_operator(binary_operator* node) {
    node->get_left()->accept(this);
    node->get_right()->accept(this);
    return true;
}

bool visitor::visit_type_convert(type_convert* node) {
    node->get_source()->accept(this);
    node->get_target()->accept(this);
    return true;
}

bool visitor::visit_identifier(identifier* node) {
    return true;
}

bool visitor::visit_nil_literal(nil_literal* node) {
    return true;
}

bool visitor::visit_number_literal(number_literal* node) {
    return true;
}

bool visitor::visit_string_literal(string_literal* node) {
    return true;
}

bool visitor::visit_char_literal(char_literal* node) {
    return true;
}

bool visitor::visit_bool_literal(bool_literal* node) {
    return true;
}

bool visitor::visit_array_list(array_list* node) {
    for (auto i : node->get_value()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_call_id(call_id* node) {
    node->get_id()->accept(this);
    if (node->get_generic_types()) {
        node->get_generic_types()->accept(this);
    }
    return true;
}

bool visitor::visit_call_index(call_index* node) {
    node->get_index()->accept(this);
    return true;
}

bool visitor::visit_call_func_args(call_func_args* node) {
    for (auto i : node->get_args()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_get_field(get_field* node) {
    return true;
}

bool visitor::visit_ptr_get_field(ptr_get_field* node) {
    return true;
}

bool visitor::visit_init_pair(init_pair* node) {
    node->get_field()->accept(this);
    node->get_value()->accept(this);
    return true;
}

bool visitor::visit_initializer(initializer* node) {
    for (auto i : node->get_pairs()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_call_path(call_path* node) {
    return true;
}

bool visitor::visit_call(call* node) {
    node->get_head()->accept(this);
    for (auto i : node->get_chain()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_assignment(assignment* node) {
    node->get_left()->accept(this);
    node->get_right()->accept(this);
    return true;
}

bool visitor::visit_stmt(stmt* node) {
    node->accept(this);
    return true;
}

bool visitor::visit_use_stmt(use_stmt* node) {
    for (auto i : node->get_module_path()) {
        i->accept(this);
    }
    for (auto i : node->get_import_symbol()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_definition(definition* node) {
    if (node->get_type()) {
        node->get_type()->accept(this);
    }
    if (node->get_init_value()) {
        node->get_init_value()->accept(this);
    }
    return true;
}

bool visitor::visit_cond_stmt(cond_stmt* node) {
    for (auto i : node->get_stmts()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_if_stmt(if_stmt* node) {
    if (node->get_condition()) {
        node->get_condition()->accept(this);
    }
    node->get_block()->accept(this);
    return true;
}

bool visitor::visit_match_case(match_case* node) {
    node->get_value()->accept(this);
    node->get_block()->accept(this);
    return true;
}

bool visitor::visit_match_stmt(match_stmt* node) {
    node->get_value()->accept(this);
    for (auto i : node->get_cases()) {
        i->accept(this);
    }
    return true;
}

bool visitor::visit_while_stmt(while_stmt* node) {
    node->get_condition()->accept(this);
    node->get_block()->accept(this);
    return true;
}

bool visitor::visit_for_stmt(for_stmt* node) {
    if (node->get_init()) {
        node->get_init()->accept(this);
    }
    if (node->get_condition()) {
        node->get_condition()->accept(this);
    }
    if (node->get_update()) {
        node->get_update()->accept(this);
    }
    node->get_block()->accept(this);
    return true;
}

bool visitor::visit_forindex(forindex* node) {
    if (node->get_variable()) {
        node->get_variable()->accept(this);
    }
    if (node->get_container()) {
        node->get_container()->accept(this);
    }
    node->get_body()->accept(this);
    return true;
}

bool visitor::visit_foreach(foreach* node) {
    if (node->get_variable()) {
        node->get_variable()->accept(this);
    }
    if (node->get_container()) {
        node->get_container()->accept(this);
    }
    node->get_body()->accept(this);
    return true;
}

bool visitor::visit_in_stmt_expr(in_stmt_expr* node) {
    node->get_expr()->accept(this);
    return true;
}

bool visitor::visit_defer_stmt(defer_stmt* node) {
    if (node->get_block()) {
        node->get_block()->accept(this);
    }
    return true;
}

bool visitor::visit_ret_stmt(ret_stmt* node) {
    if (node->get_value()) {
        node->get_value()->accept(this);
    }
    return true;
}

bool visitor::visit_continue_stmt(continue_stmt* node) {
    return true;
}

bool visitor::visit_break_stmt(break_stmt* node) {
    return true;
}

bool visitor::visit_code_block(code_block* node) {
    for (auto i : node->get_stmts()) {
        i->accept(this);
    }
    return true;
}

}