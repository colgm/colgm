#include "ast/dumper.h"

namespace colgm::ast {

bool dumper::visit_root(root* node) {
    dump_indent();
    std::cout << "root" << format_location(node);
    push_indent();
    for(auto i : node->get_use_stmts()) {
        if (i==node->get_use_stmts().back() && node->get_decls().empty()) {
            set_last();
        }
        i->accept(this);
    }
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
    std::cout << "identifier " << node->get_name() << format_location(node);
    return true;
}

bool dumper::visit_nil_literal(nil_literal* node) {
    dump_indent();
    std::cout << "nil" << format_location(node);
    return true;
}

bool dumper::visit_number_literal(number_literal* node) {
    dump_indent();
    std::cout << "number " << node->get_number() << format_location(node);
    return true;
}

bool dumper::visit_string_literal(string_literal* node) {
    dump_indent();
    std::cout << "string \"" << llvm_raw_string(node->get_string()) << "\"";
    std::cout << format_location(node);
    return true;
}

bool dumper::visit_char_literal(char_literal* node) {
    dump_indent();
    std::string literal = std::string(1, node->get_char());
    std::cout << "char " << llvm_raw_string(literal) << format_location(node);
    return true;
}

bool dumper::visit_bool_literal(bool_literal* node) {
    dump_indent();
    std::cout << "bool " << (node->get_flag()? "true":"false") << format_location(node);
    return true;
}

bool dumper::visit_array_literal(array_literal* node) {
    dump_indent();
    std::cout << "array" << format_location(node);
    push_indent();
    node->get_size()->accept(this);
    set_last();
    node->get_type()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_condition_comment(condition_comment* node) {
    dump_indent();
    std::cout << "condition_comment " << node->get_condition_name();
    std::cout << format_location(node);
    return true;
}

bool dumper::visit_type_def(type_def* node) {
    dump_indent();
    std::cout << "type " << (node->is_constant()? "[const]":"[mut]");
    std::cout << "[ptr " << node->get_pointer_level() << "] ";
    std::cout << node->get_name()->get_name();
    std::cout << format_location(node);
    push_indent();
    if (node->get_generic_types()) {
        set_last();
        node->get_generic_types()->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_generic_type_list(generic_type_list* node) {
    dump_indent();
    std::cout << "generics" << format_location(node);
    push_indent();
    for(auto i : node->get_types()) {
        if (i==node->get_types().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_enum_decl(enum_decl* node) {
    dump_indent();
    std::cout << "enum ";
    if (node->is_public_enum()) {
        std::cout << "[pub]";
    }
    std::cout << node->get_name()->get_name() << format_location(node);
    push_indent();
    if (node->get_member().empty()) {
        set_last();
    }
    for(auto i : node->get_member()) {
        if (i.name==node->get_member().back().name) {
            set_last();
        }
        i.name->accept(this);
        if (i.value) {
            push_indent();
            set_last();
            i.value->accept(this);
            pop_indent();
        }
    }
    pop_indent();
    return true;
}

bool dumper::visit_struct_field(struct_field* node) {
    dump_indent();
    std::cout << "field" << format_location(node);
    push_indent();
    node->get_name()->accept(this);
    set_last();
    node->get_type()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_struct_decl(struct_decl* node) {
    dump_indent();
    std::cout << "struct ";
    if (node->is_public_struct()) {
        std::cout << "[pub]";
    }
    if (node->is_extern_struct()) {
        std::cout << "[extern]";
    }
    if (node->is_public_struct() || node->is_extern_struct()) {
        std::cout << " ";
    }
    std::cout << node->get_name() << format_location(node);
    push_indent();
    if (node->get_generic_types()) {
        if (node->get_fields().empty()) {
            set_last();
        }
        node->get_generic_types()->accept(this);
    }
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
    std::cout << "param " << node->get_name()->get_name();
    std::cout << format_location(node);
    push_indent();
    if (node->get_type()) {
        set_last();
        node->get_type()->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_param_list(param_list* node) {
    dump_indent();
    std::cout << "params" << format_location(node);
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
    std::cout << "func ";
    if (node->is_public_func()) {
        std::cout << "[pub]";
    }
    if (node->is_extern_func()) {
        std::cout << "[extern]";
    }
    if (node->is_public_func() || node->is_extern_func()) {
        std::cout << " ";
    }
    std::cout << node->get_name() << format_location(node);
    push_indent();
    if (node->get_generic_types()) {
        node->get_generic_types()->accept(this);
    }
    node->get_params()->accept(this);
    if (!node->get_code_block()) {
        set_last();
    }
    node->get_return_type()->accept(this);
    set_last();
    if (node->get_code_block()) {
        node->get_code_block()->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_impl_struct(impl_struct* node) {
    dump_indent();
    std::cout << "impl " << node->get_struct_name() << format_location(node);
    push_indent();
    if (node->get_generic_types()) {
        if (node->get_methods().empty()) {
            set_last();
        }
        node->get_generic_types()->accept(this);
    }
    for(auto i : node->get_methods()) {
        if (i==node->get_methods().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_unary_operator(unary_operator* node) {
    dump_indent();
    std::cout << "unary ";
    switch(node->get_opr()) {
        case unary_operator::kind::neg: std::cout << "-"; break;
        case unary_operator::kind::bnot: std::cout << "~"; break;
        case unary_operator::kind::lnot: std::cout << "!"; break;
    }
    std::cout << format_location(node);
    push_indent();
    set_last();
    node->get_value()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_binary_operator(binary_operator* node) {
    dump_indent();
    std::cout << "binary ";
    switch(node->get_opr()) {
        case binary_operator::kind::add: std::cout << "+"; break;
        case binary_operator::kind::sub: std::cout << "-"; break;
        case binary_operator::kind::rem: std::cout << "%"; break;
        case binary_operator::kind::mult: std::cout << "*"; break;
        case binary_operator::kind::div: std::cout << "/"; break;
        case binary_operator::kind::cmpeq: std::cout << "=="; break;
        case binary_operator::kind::cmpneq: std::cout << "!="; break;
        case binary_operator::kind::less: std::cout << "<"; break;
        case binary_operator::kind::leq: std::cout << "<="; break;
        case binary_operator::kind::grt: std::cout << ">"; break;
        case binary_operator::kind::geq: std::cout << ">="; break;
        case binary_operator::kind::cmpand: std::cout << "and"; break;
        case binary_operator::kind::cmpor: std::cout << "or"; break;
        case binary_operator::kind::band: std::cout << "&"; break;
        case binary_operator::kind::bxor: std::cout << "^"; break;
        case binary_operator::kind::bor: std::cout << "|"; break;
    }
    std::cout << format_location(node);
    push_indent();
    node->get_left()->accept(this);
    set_last();
    node->get_right()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_type_convert(type_convert* node) {
    dump_indent();
    std::cout << "type_convert" << format_location(node);
    push_indent();
    node->get_source()->accept(this);
    set_last();
    node->get_target()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_call_id(call_id* node) {
    dump_indent();
    std::cout << "call_id " << node->get_id()->get_name();
    std::cout << format_location(node);
    if (node->get_generic_types()) {
        push_indent();
        set_last();
        node->get_generic_types()->accept(this);
        pop_indent();
    }
    return true;
}

bool dumper::visit_call_index(call_index* node) {
    dump_indent();
    std::cout << "call_index" << format_location(node);
    push_indent();
    set_last();
    node->get_index()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_call_func_args(call_func_args* node) {
    dump_indent();
    std::cout << "call_func_args" << format_location(node);
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

bool dumper::visit_get_field(get_field* node) {
    dump_indent();
    std::cout << "call_field @" << node->get_name() << format_location(node);
    return true;
}

bool dumper::visit_ptr_get_field(ptr_get_field* node) {
    dump_indent();
    std::cout << "ptr_call_field @" << node->get_name() << format_location(node);
    return true;
}

bool dumper::visit_init_pair(init_pair* node) {
    dump_indent();
    std::cout << "initializer_pair" << format_location(node);
    push_indent();
    node->get_field()->accept(this);
    set_last();
    node->get_value()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_initializer(initializer* node) {
    dump_indent();
    std::cout << "initializer" << format_location(node);
    push_indent();
    for(auto i : node->get_pairs()) {
        if (i==node->get_pairs().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_call_path(call_path* node) {
    dump_indent();
    std::cout << "call_path @" << node->get_name() << format_location(node);
    return true;
}

bool dumper::visit_call(call* node) {
    dump_indent();
    std::cout << "call" << format_location(node);
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

bool dumper::visit_assignment(assignment* node) {
    dump_indent();
    std::cout << "assignment ";
    switch(node->get_type()) {
        case assignment::kind::eq: std::cout << "="; break;
        case assignment::kind::addeq: std::cout << "+= "; break;
        case assignment::kind::diveq: std::cout << "/= "; break;
        case assignment::kind::remeq: std::cout << "%= "; break;
        case assignment::kind::multeq: std::cout << "*= "; break;
        case assignment::kind::subeq: std::cout << "-= "; break;
        case assignment::kind::andeq: std::cout << "&= "; break;
        case assignment::kind::xoreq: std::cout << "^= "; break;
        case assignment::kind::oreq: std::cout << "|= "; break;
    }
    std::cout << format_location(node);
    push_indent();
    node->get_left()->accept(this);
    set_last();
    node->get_right()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_use_stmt(use_stmt* node) {
    dump_indent();
    std::cout << "use" << format_location(node);
    push_indent();
    for(auto i : node->get_module_path()) {
        i->accept(this);
    }
    set_last();
    dump_indent();
    if (node->get_import_symbol().empty()) {
        std::cout << "import_all\n";
    } else {
        std::cout << "import_symbol\n";
        push_indent();
        for(auto i : node->get_import_symbol()) {
            if (i==node->get_import_symbol().back()) {
                set_last();
            }
            i->accept(this);
        }
        pop_indent();
    }
    pop_indent();
    return true;
}

bool dumper::visit_definition(definition* node) {
    dump_indent();
    std::cout << "define @" << node->get_name() << " ";
    std::cout << format_location(node);
    push_indent();
    if (node->get_type() && !node->get_init_value()) {
        set_last();
    }
    if (node->get_type()) {
        node->get_type()->accept(this);
    }
    set_last();
    if (node->get_init_value()) {
        node->get_init_value()->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_cond_stmt(cond_stmt* node) {
    dump_indent();
    std::cout << "condition" << format_location(node);
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

bool dumper::visit_if_stmt(if_stmt* node) {
    dump_indent();
    if (node->get_condition()) {
        std::cout << "if";
    } else {
        std::cout << "else";
    }
    std::cout << "_statement" << format_location(node);
    push_indent();
    if (node->get_condition()) {
        node->get_condition()->accept(this);
    }
    set_last();
    node->get_block()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_match_case(match_case* node) {
    dump_indent();
    std::cout << "case" << format_location(node);
    push_indent();
    node->get_value()->accept(this);
    set_last();
    node->get_block()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_match_stmt(match_stmt* node) {
    dump_indent();
    std::cout << "match" << format_location(node);
    push_indent();
    if (node->get_cases().empty()) {
        set_last();
    }
    node->get_value()->accept(this);

    push_indent();
    for(auto i : node->get_cases()) {
        if (i==node->get_cases().back()) {
            set_last();
        }
        i->accept(this);
    }
    pop_indent();

    pop_indent();
    return true;
}

bool dumper::visit_while_stmt(while_stmt* node) {
    dump_indent();
    std::cout << "while" << format_location(node);
    push_indent();
    node->get_condition()->accept(this);
    set_last();
    node->get_block()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_for_stmt(for_stmt* node) {
    dump_indent();
    std::cout << "for" << format_location(node);
    push_indent();
    if (node->get_init()) {
        node->get_init()->accept(this);
    }
    if (node->get_condition()) {
        node->get_condition()->accept(this);
    }
    if (node->get_update()) {
        node->get_update()->accept(this);
    }
    set_last();
    node->get_block()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_in_stmt_expr(in_stmt_expr* node) {
    dump_indent();
    std::cout << "in_statement_expression" << format_location(node);
    push_indent();
    set_last();
    node->get_expr()->accept(this);
    pop_indent();
    return true;
}

bool dumper::visit_ret_stmt(ret_stmt* node) {
    dump_indent();
    std::cout << "return" << format_location(node);
    push_indent();
    set_last();
    if (node->get_value()) {
        node->get_value()->accept(this);
    }
    pop_indent();
    return true;
}

bool dumper::visit_continue_stmt(continue_stmt* node) {
    dump_indent();
    std::cout << "continue" << format_location(node);
    return true;
}

bool dumper::visit_break_stmt(break_stmt* node) {
    dump_indent();
    std::cout << "break" << format_location(node);
    return true;
}

bool dumper::visit_code_block(code_block* node) {
    dump_indent();
    std::cout << "block" << format_location(node);
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