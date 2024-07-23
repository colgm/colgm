#pragma once

#include "mir/visitor.h"
#include "mir/ast2mir.h"
#include "code/sir.h"
#include "code/context.h"

namespace colgm::mir {

class mir2sir: public visitor {
private:
    ir_context ictx;
    sir_block* block;

private:
    void emit_struct(const mir_context&);
    void emit_func_decl(const mir_context&);
    void emit_func_impl(const mir_context&);

private:
    void visit_mir_define(mir_define*) override;

public:
    mir2sir(): block(nullptr) {}
    void generate(const mir_context&);
};

}
