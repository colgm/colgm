#include "sema/basic.h"

namespace colgm {

std::unordered_map<std::string, colgm_basic*> colgm_basic::mapper = {
    {"u8", colgm_basic::u8()},
    {"u16", colgm_basic::u16()},
    {"u32", colgm_basic::u32()},
    {"u64", colgm_basic::u64()},
    {"i8", colgm_basic::i8()},
    {"i16", colgm_basic::i16()},
    {"i32", colgm_basic::i32()},
    {"i64", colgm_basic::i64()},
    {"f32", colgm_basic::f32()},
    {"f64", colgm_basic::f64()}
};

colgm_basic* colgm_basic::u8() {
    static colgm_basic singleton = colgm_basic {"u8", {}, {}};
    return &singleton;
}

colgm_basic* colgm_basic::u16() {
    static colgm_basic singleton = colgm_basic {"u16", {}, {}};
    return &singleton;
}

colgm_basic* colgm_basic::u32() {
    static colgm_basic singleton = colgm_basic {"u32", {}, {}};
    return &singleton;
}

colgm_basic* colgm_basic::u64() {
    static colgm_basic singleton = colgm_basic {"u64", {}, {}};
    return &singleton;
}

colgm_basic* colgm_basic::i8() {
    static colgm_basic singleton = colgm_basic {"i8", {}, {}};
    return &singleton;
}

colgm_basic* colgm_basic::i16() {
    static colgm_basic singleton = colgm_basic {"i16", {}, {}};
    return &singleton;
}

colgm_basic* colgm_basic::i32() {
    static colgm_basic singleton = colgm_basic {"i32", {}, {}};
    return &singleton;
}

colgm_basic* colgm_basic::i64() {
    static colgm_basic singleton = colgm_basic {"i64", {}, {}};
    return &singleton;
}

colgm_basic* colgm_basic::f32() {
    static colgm_basic singleton = colgm_basic {"f32", {}, {}};
    return &singleton;
}

colgm_basic* colgm_basic::f64() {
    static colgm_basic singleton = colgm_basic {"f64", {}, {}};
    return &singleton;
}

}