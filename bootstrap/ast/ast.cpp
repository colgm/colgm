#include "ast/ast.h"
#include "ast/visitor.h"

namespace colgm::ast {

root::~root() {
    for (auto i : imports) {
        delete i;
    }
    for (auto i : declarations) {
        delete i;
    }
}

root* root::clone() const {
    auto ret = new root(location);
    for (auto i : imports) {
        ret->add_use_stmt(i->clone());
    }
    for (auto i : declarations) {
        ret->add_decl(i->clone());
    }
    return ret;
}

void root::accept(visitor* v) {
    v->visit_root(this);
}

}