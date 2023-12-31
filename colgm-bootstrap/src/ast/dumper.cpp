#include "ast/dumper.h"

namespace colgm {

bool dumper::visit_root(root* node) {
    dump_indent();
    std::cout << "root " << format_location(node->get_location());
    push_indent();
    for(auto i : node->get_decls()) {
        if (i==node->get_decls().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_identifier(identifier* node) {
    dump_indent();
    std::cout << "identifier " << node->get_name() << format_location(node->get_location());
    return true;
}

bool dumper::visit_number_literal(number_literal* node) {
    dump_indent();
    std::cout << "number " << node->get_number() << format_location(node->get_location());
    return true;
}

bool dumper::visit_string_literal(string_literal* node) {
    dump_indent();
    std::cout << "string " << node->get_string() << format_location(node->get_location());
    return true;
}

bool dumper::visit_type_def(type_def* node) {
    dump_indent();
    std::cout << "type ptr " << node->get_pointer_level() << format_location(node->get_location());
    push_indent();
    set_last();
    node->get_name()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_struct_field(struct_field* node) {
    dump_indent();
    std::cout << "field " << format_location(node->get_location());
    push_indent();
    node->get_name()->accept(this);
    set_last();
    node->get_type()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_struct_decl(struct_decl* node) {
    dump_indent();
    std::cout << "struct " << node->get_name() << format_location(node->get_location());
    push_indent();
    for(auto i : node->get_fields()) {
        if (i==node->get_fields().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_param(param* node) {
    dump_indent();
    std::cout << "param " << format_location(node->get_location());
    push_indent();
    node->get_name()->accept(this);
    set_last();
    node->get_type()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_param_list(param_list* node) {
    dump_indent();
    std::cout << "param list " << format_location(node->get_location());
    push_indent();
    for(auto i : node->get_params()) {
        if (i==node->get_params().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_func_decl(func_decl* node) {
    dump_indent();
    std::cout << "func " << node->get_name() << format_location(node->get_location());
    push_indent();
    node->get_params()->accept(this);
    node->get_return_type()->accept(this);
    set_last();
    node->get_code_block()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_binary_operator(binary_operator* node) {
    dump_indent();
    std::cout << "binary ";
    switch(node->get_opr()) {
        case binary_operator::kind::add: std::cout << "+"; break;
        case binary_operator::kind::sub: std::cout << "-"; break;
        case binary_operator::kind::mod: std::cout << "%"; break;
        case binary_operator::kind::mult: std::cout << "*"; break;
        case binary_operator::kind::div: std::cout << "/"; break;
    }
    std::cout << " " << format_location(node->get_location());
    push_indent();
    node->get_left()->accept(this);
    set_last();
    node->get_right()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_call_index(call_index* node) {
    dump_indent();
    std::cout << "call index " << format_location(node->get_location());
    push_indent();
    set_last();
    node->get_index()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_call_func_args(call_func_args* node) {
    dump_indent();
    std::cout << "call func " << format_location(node->get_location());
    push_indent();
    for(auto i : node->get_args()) {
        if (i==node->get_args().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_call_field(call_field* node) {
    dump_indent();
    std::cout << "call field @" << node->get_name() << " " << format_location(node->get_location());
    return true;
}

bool dumper::visit_call(call* node) {
    dump_indent();
    std::cout << "call " << format_location(node->get_location());
    push_indent();
    if (node->get_chain().empty()) {
        set_last();
    }
    node->get_head()->accept(this);
    for(auto i : node->get_chain()) {
        if (i==node->get_chain().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_in_stmt_expr(in_stmt_expr* node) {
    dump_indent();
    std::cout << "in statement " << format_location(node->get_location());
    push_indent();
    set_last();
    node->get_expr()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_ret_stmt(ret_stmt* node) {
    dump_indent();
    std::cout << "return " << format_location(node->get_location());
    push_indent();
    set_last();
    if (node->get_value()) {
        node->get_value()->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_code_block(code_block* node) {
    dump_indent();
    std::cout << "code block " << format_location(node->get_location());
    push_indent();
    for(auto i : node->get_stmts()) {
        if (i==node->get_stmts().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();
    return true;
}

}