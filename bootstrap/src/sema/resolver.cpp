#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "sema/resolver.h"

namespace colgm {

type resolver::resolve_type_def(ast::type_def* node) {
    const auto& name = node->get_name()->get_name();

    // cannot find type
    if (!ctx.global_symbol.count(name) &&
        !ctx.generics.count(name)) {
        rp.report(node->get_name(),
            "unknown type \"" + name + "\"."
        );
        return type::error_type();
    }

    // find type but type is not imported as public
    // mainly eccurs when import all symbols from a module
    // report this error, but still resolve the type
    if (ctx.global_symbol.count(name) &&
        !ctx.global_symbol.at(name).is_public) {
        rp.report(node->get_name(),
            "private type \"" + name + "\" cannot be used."
        );
    }

    // generate resolved type
    auto res = type {
        .name = name,
        .loc_file = ctx.global_symbol.count(name)
            ? ctx.global_symbol.at(name).loc_file
            : "",
        .pointer_depth = node->get_pointer_level()
    };

    // if node has const flag, set it
    if (node->is_constant()) {
        res.is_constant_type = true;
    }

    // if node has generics, set it
    if (ctx.generics.size() && ctx.generics.count(name)) {
        res.is_generic_placeholder = true;
    }
    return res;
}

}