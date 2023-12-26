#include "ast/expr.h"
#include "ast/visitor.h"

namespace colgm {

void expr::accept(visitor* v) {
    v->visit_expr(this);
}

void identifier::accept(visitor* v) {
    v->visit_identifier(this);
}

}