#include "ast/ast.h"
#include "ast/visitor.h"

namespace colgm {

root::~root() {
    for(auto i : use_statements) {
        delete i;
    }
    for(auto i : declarations) {
        delete i;
    }
}

void root::accept(visitor* v) {
    v->visit_root(this);
}

}