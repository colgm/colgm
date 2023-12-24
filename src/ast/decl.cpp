#include "ast/decl.h"
#include "ast/visitor.h"

namespace colgm {

void decl::accept(visitor* v) {
    v->visit_decl(this);
}

}