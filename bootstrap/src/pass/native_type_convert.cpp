#include "pass/native_type_convert.h"

namespace colgm {

void native_type_convert::gen(const std::string& source,
                              const std::unordered_set<std::string>& targets) {
    for(const auto& target : targets) {
        auto convert_function = new hir_func(
            "@native." + source + "." + target
        );
        convert_function->add_param("num", type_mapper.at(source));
        convert_function->set_return_type(type_mapper.at(target));
        convert_function->set_code_block(new sir_block);
        if (type_mapper.at(source)==type_mapper.at(target)) {
            convert_function->get_code_block()->add_stmt(new sir_ret(
                type_mapper.at(target),
                value_t::variable("num")
            ));
        } else {
            convert_function->get_code_block()->add_stmt(new sir_type_convert(
                "num",
                "0",
                type_mapper.at(source),
                type_mapper.at(target),
                false
            ));
            convert_function->get_code_block()->add_stmt(new sir_ret(
                type_mapper.at(target),
                value_t::variable("0")
            ));
        }
        ctx->func_impls.push_back(convert_function);
    }
}

bool native_type_convert::run() {
    for(const auto& src : ctx->used_basic_convert_method) {
        gen(src.first, src.second);
    }
    return true;
}

}