#include "ast/stmt.h"
#include "ast/visitor.h"

namespace colgm {

void stmt::accept(visitor* v) {
    v->visit_stmt(this);
}

ret_stmt::~ret_stmt() {
    delete value;
}

void ret_stmt::accept(visitor* v) {
    v->visit_ret_stmt(this);
}

code_block::~code_block() {
    for(auto i : statements) {
        delete i;
    }
}

void code_block::accept(visitor* v) {
    v->visit_code_block(this);
}

}