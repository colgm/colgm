#pragma once

#include "report.h"
#include "ast/ast.h"

namespace colgm {

class reporter {
private:
    error& err;

public:
    reporter(error& e): err(e) {}
    bool has_error() const { return err.geterr(); }

public:
    void report(ast::node* n, const std::string& info, const std::string& note = "") {
        err.err(n->get_location(), info, note);
    }
    void report(const span& loc, const std::string& info, const std::string& note = "") {
        err.err(loc, info, note);
    }
    void warn(ast::node* n, const std::string& info, const std::string& note = "") {
        err.warn(n->get_location(), info, note);
    }
    void warn(const span& loc, const std::string& info, const std::string& note = "") {
        err.warn(loc, info, note);
    }
    void unimplemented(ast::node* n) {
        err.err(n->get_location(),
            "unimplemented, please report a bug."
        );
    }
    void unimplemented(const span& loc) {
        err.err(loc,
            "unimplemented, please report a bug."
        );
    }
    void unreachable(ast::node* n) {
        err.err(n->get_location(),
            "unreachable, please report a bug."
        );
    }
    void unreachable(const span& loc) {
        err.err(loc,
            "unreachable, please report a bug."
        );
    }
};

}