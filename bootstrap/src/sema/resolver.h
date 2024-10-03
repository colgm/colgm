#pragma once

#include "report.h"
#include "ast/ast.h"
#include "sema/context.h"
#include "sema/symbol.h"
#include "sema/reporter.h"

namespace colgm {

class resolver {
private:
    error& err;
    sema_context& ctx;
    reporter rp;

public:
    type resolve_type_def(ast::type_def*);

public:
    resolver(error& e, sema_context& c): err(e), ctx(c), rp(err) {}
};

}