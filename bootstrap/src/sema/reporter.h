#pragma once

#include "report.h"
#include "ast/ast.h"

namespace colgm {

class reporter {
private:
    error& err;

public:
    reporter(error& e): err(e) {}

public:
    void report(ast::node* n, const std::string& info) {
        err.err("semantic", n->get_location(), info);
    }
    void report(const span& loc, const std::string& info) {
        err.err("semantic", loc, info);
    }
    void warning(ast::node* n, const std::string& info) {
        err.warn("semantic", n->get_location(), info);
    }
    void warning(const span& loc, const std::string& info) {
        err.warn("semantic", loc, info);
    }
    void unimplemented(ast::node* n) {
        err.err("semantic",
            n->get_location(),
            "unimplemented, please report a bug."
        );
    }
    void unimplemented(const span& loc) {
        err.err("semantic",
            loc,
            "unimplemented, please report a bug."
        );
    }
    void unreachable(ast::node* n) {
        err.err("semantic",
            n->get_location(),
            "unreachable, please report a bug."
        );
    }
    void unreachable(const span& loc) {
        err.err("semantic",
            loc,
            "unreachable, please report a bug."
        );
    }
};

}