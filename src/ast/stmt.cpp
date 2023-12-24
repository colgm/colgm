#include "ast/stmt.h"
#include "ast/visitor.h"

namespace colgm {

void stmt::accept(visitor* v) {
    v->visit_stmt(this);
}

use_stmt::~use_stmt() {
    for(auto i : path) {
        delete i;
    }
    delete single_use;
    for(auto i : specified_use) {
        delete i;
    }
}

void use_stmt::accept(visitor* v) {
    v->visit_use_stmt(this);
}

}