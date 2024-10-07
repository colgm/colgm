#pragma once

#include "report.h"
#include "ast/ast.h"
#include "sema/context.h"
#include "sema/symbol.h"
#include "sema/reporter.h"

namespace colgm {

class type_resolver {
private:
    error& err;
    sema_context& ctx;
    reporter rp;

public:
    type_resolver(error& e, sema_context& c):
        err(e), ctx(c), rp(err) {}
    type resolve(ast::type_def*);
};

}