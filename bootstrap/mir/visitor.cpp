#include "mir/visitor.h"

namespace colgm::mir {

void visitor::visit_mir_block(mir_block* node) {
    for (auto i : node->get_content()) {
        i->accept(this);
    }
}

void visitor::visit_mir_unary(mir_unary* node) {
    node->get_value()->accept(this);
}

void visitor::visit_mir_binary(mir_binary* node) {
    node->get_left()->accept(this);
    node->get_right()->accept(this);
}

void visitor::visit_mir_type_convert(mir_type_convert* node) {
    node->get_source()->accept(this);
}

void visitor::visit_mir_array(mir_array* node) {
    if (node->get_value()) {
        node->get_value()->accept(this);
    }
}

void visitor::visit_mir_struct_init(mir_struct_init* node) {
    for (const auto& i : node->get_fields()) {
        i.content->accept(this);
    }
}

void visitor::visit_mir_call(mir_call* node) {
    node->get_content()->accept(this);
}

void visitor::visit_mir_call_index(mir_call_index* node) {
    node->get_index()->accept(this);
}

void visitor::visit_mir_call_func(mir_call_func* node) {
    node->get_args()->accept(this);
}

void visitor::visit_mir_define(mir_define* node) {
    node->get_init_value()->accept(this);
}

void visitor::visit_mir_assign(mir_assign* node) {
    node->get_left()->accept(this);
    node->get_right()->accept(this);
}

void visitor::visit_mir_if(mir_if* node) {
    if (node->get_condition()) {
        node->get_condition()->accept(this);
    }
    node->get_content()->accept(this);
}

void visitor::visit_mir_branch(mir_branch* node) {
    for (auto i : node->get_branch()) {
        i->accept(this);
    }
}

void visitor::visit_mir_switch_case(mir_switch_case* node) {
    node->get_content()->accept(this);
}

void visitor::visit_mir_switch(mir_switch* node) {
    if (node->get_condition()) {
        node->get_condition()->accept(this);
    }
    for (auto i : node->get_cases()) {
        i->accept(this);
    }
    if (node->get_default_case()) {
        node->get_default_case()->accept(this);
    }
}

void visitor::visit_mir_loop(mir_loop* node) {
    node->get_condition()->accept(this);
    node->get_content()->accept(this);
    if (node->get_update()) {
        node->get_update()->accept(this);
    }
}

void visitor::visit_mir_return(mir_return* node) {
    node->get_value()->accept(this);
}

}
