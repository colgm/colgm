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

bool dumper::visit_type_def(type_def* node) {
    dump_indent();
    std::cout << "type ptr " << node->get_pointer_level() << format_location(node->get_location());
    push_indent();
    set_last();
    node->get_name()->accept(this);
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

bool dumper::visit_code_block(code_block* node) {
    // TODO
    dump_indent();
    std::cout << "code block " << format_location(node->get_location());
    return true;
}

}