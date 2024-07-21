#include "mir/type_cast_number_pass.h"

namespace colgm::mir {

mir_number* type_cast_number::do_optimize(mir_type_convert* node) {
    auto ir_node = node->get_source()->get_content().front();
    auto num_node = reinterpret_cast<mir_number*>(ir_node);
    return new mir_number(
        num_node->get_location(),
        num_node->get_literal(),
        node->get_target_type()
    );
}

bool type_cast_number::cast_const_number(mir_type_convert* node) {
    if (!node->get_source()->get_content().size() ||
        node->get_source()->get_content().size()>1) {
        return false;
    }
    auto ir_node = node->get_source()->get_content().front();
    return ir_node->get_kind()==kind::mir_number;
}

void type_cast_number::visit_mir_block(mir_block* node) {
    for(auto& i : node->get_mutable_content()) {
        i->accept(this);
        if (i->get_kind()==kind::mir_type_convert &&
            cast_const_number(reinterpret_cast<mir_type_convert*>(i))) {
            auto temp = i;
            i = do_optimize(reinterpret_cast<mir_type_convert*>(i));
            delete temp;
        }
    }
    return;
}

bool type_cast_number::run(mir_context* mctx) {
    for(auto i : mctx->impls) {
        i->block->accept(this);
    }
    return true;
}

}
