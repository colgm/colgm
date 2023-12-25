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
    delete specified;
}

void use_stmt::accept(visitor* v) {
    v->visit_use_stmt(this);
}

specified_use::~specified_use() {
    for(auto i : symbols) {
        delete i;
    }
}

void specified_use::accept(visitor* v) {
    v->visit_specified_use(this);
}

}