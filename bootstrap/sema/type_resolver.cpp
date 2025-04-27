#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "sema/type_resolver.h"

#include <cassert>
#include <vector>

namespace colgm {

bool type_resolver::is_generic(const type& t) {
    if (t.is_generic_placeholder) {
        return true;
    }
    for (auto& g : t.generics) {
        if (is_generic(g)) {
            return true;
        }
    }
    return false;
}

u64 type_resolver::get_array_length(ast::number_literal* n) {
    const auto& literal_string = n->get_number();

    if (literal_string.find(".") != std::string::npos ||
        literal_string.find("e") != std::string::npos) {
        rp.report(n, "invalid float number \"" + literal_string + "\", expect u64.");
        return 0;
    }

    u64 res = 0;
    if (literal_string.length() > 2 && literal_string[1] == 'o') {
        res = oct_to_u64(literal_string.c_str());
    } else if (literal_string.length() > 2 && literal_string[1] == 'x') {
        res = hex_to_u64(literal_string.c_str());
    } else {
        res = dec_to_u64(literal_string.c_str());
    }

    if (res == 0) {
        rp.report(n, "invalid number \"" + literal_string + "\", length should not be 0.");
    }
    if (res < 0 || res >= UINT32_MAX) {
        rp.report(n,
            "invalid number \"" + literal_string +
            "\", length should be 0 ~ " + std::to_string(UINT32_MAX) + "."
        );
    }
    return res;
}

type type_resolver::resolve(ast::type_def* node) {
    const auto& name = node->get_name()->get_name();
    const auto& dm = node->is_redirected()
        ? ctx.global.domain.at(node->get_redirect_location())
        : ctx.global.domain.at(node->get_file());

    // cannot find type
    if (!dm.global_symbol.count(name) &&
        !dm.generic_symbol.count(name) &&
        !ctx.generics.count(name)) {
        rp.report(node->get_name(),
            "undefined type \"" + name + "\"."
        );
        return type::error_type();
    }

    // find type but type is not imported as public
    // mainly occurs when import all symbols from a module
    if (dm.global_symbol.count(name) &&
        !dm.global_symbol.at(name).is_public) {
        rp.report(node->get_name(),
            "private type \"" + name + "\" cannot be used."
        );
    }

    // function cannot be used as type
    if (dm.global_symbol.count(name) &&
        dm.global_symbol.at(name).kind == sym_kind::func_kind) {
        rp.report(node->get_name(),
            "\"" + name + "\" is a function, cannot be used as a type."
        );
    }

    // generate resolved type
    auto res = type {
        .name = name,
        .loc_file = dm.global_symbol.count(name)
            ? dm.global_symbol.at(name).loc_file
            : "",
        .pointer_depth = node->get_pointer_level()
    };

    // if node has const flag, set it
    if (node->is_constant()) {
        res.is_const = true;
    }
    if (node->get_is_array()) {
        res.is_array = true;
        res.pointer_depth ++;
        res.array_length = get_array_length(node->get_array_length());
    }

    // if node has generics and we can find it, set it as the place holder
    if (ctx.generics.size() && ctx.generics.count(name)) {
        res.is_generic_placeholder = true;
    }

    // resolve generics
    if (node->get_generic_types()) {
        res.generics = {};

        for (auto& g : node->get_generic_types()->get_types()) {
            res.generics.push_back(resolve(g));
        }
    }

    if (!is_generic(res)) {
        ctx.global.all_used_types[res.full_path_name_with_pointer()] = res;
    }
    return res;
}

}