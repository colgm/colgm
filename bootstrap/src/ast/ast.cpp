#include "ast/ast.h"
#include "ast/visitor.h"

namespace colgm::ast {

root::~root() {
    for(auto i : imports) {
        delete i;
    }
    for(auto i : declarations) {
        delete i;
    }
}

root* root::clone() const {
    return new root(span::null());
}

void root::accept(visitor* v) {
    v->visit_root(this);
}

}