#include "ast/dumper.h"

namespace colgm {

bool dumper::visit_root(root* node) {
    dump_indent();
    std::cout << "root " << format_location(node->get_location());
    push_indent();
    for(auto i : node->get_use_statements()) {
        if (i==node->get_use_statements().back() &&
            node->get_declarations().empty()) {
            set_last();
        }
        i->accept(this);
    }
    for(auto i : node->get_declarations()) {
        if (i==node->get_declarations().back()) {
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

bool dumper::visit_use_stmt(use_stmt* node) {
    dump_indent();
    std::cout << "use statement " << format_location(node->get_location());
    push_indent();
    for(auto i : node->get_path()) {
        i->accept(this);
    }
    set_last();
    switch(node->get_use_kind()) {
        case use_stmt::use_kind::use_all:
            dump_indent();
            std::cout << "use all\n";
            break;
        case use_stmt::use_kind::use_specify:
            node->get_specified()->accept(this);
            break;
    }
    pop_indent();
    return true;
}

bool dumper::visit_specified_use(specified_use* node) {
    dump_indent();
    std::cout << "specified use " << format_location(node->get_location());
    push_indent();
    for(auto i : node->get_symbols()) {
        if (i==node->get_symbols().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();
    return true;
}

}