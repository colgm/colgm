#pragma once

#include "sir/pass_manager.h"

#include <cstring>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

namespace colgm {

class primitive_size_opt: public sir_pass {
private:
    std::unordered_map<std::string, std::string> primitive_methods = {
        {"i8.__size__", "1"}, {"i16.__size__", "2"},
        {"i32.__size__", "4"}, {"i64.__size__", "8"},
        {"u8.__size__", "1"}, {"u16.__size__", "2"},
        {"u32.__size__", "4"}, {"u64.__size__", "8"},
        {"f32.__size__", "4"}, {"f64.__size__", "8"},
        {"bool.__size__", "1"}
    };
    u64 replace_count;

private:
    void remove_primitive_size_method(sir_block*);

public:
    primitive_size_opt(): sir_pass(), replace_count(0) {}
    ~primitive_size_opt() override = default;
    std::string name() override {
        return "primitive size opt";
    }
    std::string info() override {
        return "replace " +
               std::to_string(replace_count) +
               " primitive size method call" +
               (replace_count > 1 ? "s" : "");
    }
    bool run(sir_context*) override;
};

}