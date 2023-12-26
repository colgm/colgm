#include "ast/stmt.h"
#include "ast/visitor.h"

namespace colgm {

void stmt::accept(visitor* v) {
    v->visit_stmt(this);
}

void code_block::accept(visitor* v) {
    v->visit_code_block(this);
}

}