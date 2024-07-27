#pragma once

#include "report.h"
#include "sema/symbol.h"
#include "sema/context.h"
#include "mir/visitor.h"
#include "mir/ast2mir.h"
#include "code/sir.h"
#include "code/context.h"

#include <unordered_map>
#include <cstring>
#include <sstream>
#include <vector>

namespace colgm::mir {

class ssa_generator {
private:
    usize counter;

public:
    ssa_generator(): counter(0) {}
    void clear() { counter = 0; }
    auto create() { return std::to_string(counter++); }
};

struct mir_value_t {
public:
    enum class kind {
        null, // reserved
        nil,
        variable,
        literal,
        func_symbol,
        struct_symbol,
        enum_symbol
    };

public:
    kind value_kind;
    std::string content;
    type resolve_type;

public:
    static auto nil(const type& ty) {
        return mir_value_t {
            .value_kind = mir_value_t::kind::nil,
            .content = "null",
            .resolve_type = ty
        };
    }
    static auto variable(const std::string& name, const type& ty) {
        return mir_value_t {
            .value_kind = mir_value_t::kind::variable,
            .content = name,
            .resolve_type = ty
        };
    }
    static auto literal(const std::string& value, const type& ty) {
        return mir_value_t {
            .value_kind = mir_value_t::kind::literal,
            .content = value,
            .resolve_type = ty
        };
    }
    static auto func_kind(const std::string& full_name, const type& ty) {
        return mir_value_t {
            .value_kind = mir_value_t::kind::func_symbol,
            .content = full_name,
            .resolve_type = ty
        };
    }
    static auto struct_kind(const std::string& full_name, const type& ty) {
        return mir_value_t {
            .value_kind = mir_value_t::kind::struct_symbol,
            .content = full_name,
            .resolve_type = ty
        };
    }
    static auto enum_kind(const std::string& full_name, const type& ty) {
        return mir_value_t {
            .value_kind = mir_value_t::kind::enum_symbol,
            .content = full_name,
            .resolve_type = ty
        };
    }
};

class mir2sir: public visitor {
private:
    const semantic_context& ctx;
    std::unordered_map<std::string, symbol_kind> type_mapper;

private:
    const std::unordered_map<std::string, std::string> basic_type_mapper = {
        {"i64", "i64"},
        {"i32", "i32"},
        {"i16", "i16"},
        {"i8", "i8"},
        {"u64", "i64"},
        {"u32", "i32"},
        {"u16", "i16"},
        {"u8", "i8"},
        {"f64", "double"},
        {"f32", "float"},
        {"void", "void"},
        {"bool", "i1"}
    };
    std::string mangle_struct(const std::string&);
    std::string type_mapping(const type&);

private:
    ir_context ictx;
    ssa_generator ssa_gen;
    sir_block* block;
    std::vector<mir_value_t> value_stack;
    error err;
    bool in_assign_lvalue_process;

private:
    void unimplemented(mir* node) {
        err.err("mir2sir",
            node->get_location(),
            "internal compiler error: unimplemented mir node."
        );
    }

private:
    void generate_type_mapper();
    void emit_struct(const mir_context&);
    void emit_func_decl(const mir_context&);
    void emit_func_impl(const mir_context&);

private:
    void visit_mir_nop(mir_nop*) override;
    void visit_mir_block(mir_block*) override;
    // void visit_mir_unary(mir_unary*) override;
    // void visit_mir_binary(mir_binary*) override;
    // void visit_mir_type_convert(mir_type_convert*) override;
    void visit_mir_nil(mir_nil*) override;
    void visit_mir_number(mir_number*) override;
    void visit_mir_string(mir_string*) override;
    void visit_mir_char(mir_char*) override;
    void visit_mir_bool(mir_bool*) override;
    void visit_mir_call(mir_call*) override;
    void visit_mir_call_id(mir_call_id*) override;
    void visit_mir_call_index(mir_call_index*) override;
    void visit_mir_call_func(mir_call_func*) override;
    void visit_mir_call_field(mir_call_field*) override;
    void visit_mir_call_path(mir_call_path*) override;
    void visit_mir_ptr_call_field(mir_ptr_call_field*) override;
    void visit_mir_define(mir_define*) override;
    void visit_mir_assign(mir_assign*) override;
    // void visit_mir_if(mir_if*) override;
    // void visit_mir_branch(mir_branch*) override;
    // void visit_mir_break(mir_break*) override;
    // void visit_mir_continue(mir_continue*) override;
    // void visit_mir_while(mir_while*) override;
    // void visit_mir_return(mir_return*) override;

public:
    mir2sir(const semantic_context& c):
        ctx(c), block(nullptr), in_assign_lvalue_process(false) {}
    const error& generate(const mir_context&);
};

}
