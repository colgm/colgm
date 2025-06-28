#include "sir/detect_redef_extern.h"

namespace colgm {

bool detect_redef_extern::run(sir_context* ctx) {
    std::unordered_set<std::string> externs = {};
    // externs should be declared only
    for (auto i : ctx->func_decls) {
        const auto name = i->get_mangled_name();
        if (externs.count(name)) {
            err.err(i->get_location(), "redefinition of extern function \"" + name + "\"");
            continue;
        }
        externs.insert(name);
    }

    return !err.geterr();
}

}