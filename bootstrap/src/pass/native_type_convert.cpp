#include "pass/native_type_convert.h"

namespace colgm {

bool native_type_convert::run() {
    static const std::unordered_map<std::string, std::string> type_mapper = {
        {"u8", "i8"}, {"u16", "i16"},
        {"u32", "i32"}, {"u64", "i64"},
        {"i8", "i8"}, {"i16", "i16"},
        {"i32", "i32"}, {"i64", "i64"},
        {"f32", "float"}, {"f64", "double"}
    };

    for(const auto& src : ctx->used_basic_convert_method) {
        for(const auto& dst : src.second) {
            auto convert_function = new hir_func(
                "@native." + src.first + "." + dst
            );
            convert_function->add_param("num", type_mapper.at(src.first));
            convert_function->set_return_type(type_mapper.at(dst));
            convert_function->set_code_block(new sir_block);
            if (type_mapper.at(src.first)==type_mapper.at(dst)) {
                convert_function->get_code_block()->add_stmt(new sir_ret(
                    type_mapper.at(dst),
                    "num"
                ));
            } else {
                convert_function->get_code_block()->add_stmt(new sir_type_convert(
                    "num",
                    "0",
                    type_mapper.at(src.first),
                    type_mapper.at(dst),
                    false
                ));
                convert_function->get_code_block()->add_stmt(new sir_ret(
                    type_mapper.at(dst),
                    "0"
                ));
            }
            ctx->func_impls.push_back(convert_function);
        }
    }
    return true;
}

}