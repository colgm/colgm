#pragma once

#include "mir/mir.h"

namespace colgm::mir {

class visitor {
public:
    virtual void visit_mir_block(mir_block*);
    virtual void visit_mir_unary(mir_unary*);
    virtual void visit_mir_binary(mir_binary*);
    virtual void visit_mir_type_convert(mir_type_convert*);
    virtual void visit_mir_nil(mir_nil*) {}
    virtual void visit_mir_number(mir_number*) {}
    virtual void visit_mir_string(mir_string*) {}
    virtual void visit_mir_char(mir_char*) {}
    virtual void visit_mir_bool(mir_bool*) {}
    virtual void visit_mir_array(mir_array*);
    virtual void visit_mir_struct_init(mir_struct_init*);
    virtual void visit_mir_call(mir_call*);
    virtual void visit_mir_call_id(mir_call_id*) {}
    virtual void visit_mir_call_index(mir_call_index*);
    virtual void visit_mir_call_func(mir_call_func*);
    virtual void visit_mir_get_field(mir_get_field*) {}
    virtual void visit_mir_get_path(mir_get_path*) {}
    virtual void visit_mir_ptr_get_field(mir_ptr_get_field*) {}
    virtual void visit_mir_define(mir_define*);
    virtual void visit_mir_assign(mir_assign*);
    virtual void visit_mir_if(mir_if*);
    virtual void visit_mir_branch(mir_branch*);
    virtual void visit_mir_switch_case(mir_switch_case*);
    virtual void visit_mir_switch(mir_switch*);
    virtual void visit_mir_break(mir_break*) {}
    virtual void visit_mir_continue(mir_continue*) {}
    virtual void visit_mir_loop(mir_loop*);
    virtual void visit_mir_return(mir_return*);
};

}
