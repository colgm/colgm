#pragma once

#include "report.h"
#include "sema/symbol.h"
#include "sema/context.h"
#include "mir/visitor.h"
#include "mir/ast2mir.h"
#include "sir/sir.h"
#include "sir/context.h"

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

struct local_table {
    std::vector<std::unordered_map<std::string, std::string>> elem;
    i64 local_scope_counter;

    local_table(): local_scope_counter(0) {}

    auto get_local(const std::string& name) {
        for (auto it = elem.rbegin(); it!=elem.rend(); ++it) {
            if (it->count(name)) {
                return it->at(name);
            }
        }
        return std::string("");
    }

    auto size() const { return elem.size(); }

    void push() { elem.push_back({}); local_scope_counter++; }
    void pop() { elem.pop_back(); }
};

struct mir_value_t {
public:
    enum class kind {
        null, // reserved
        nil,
        variable,
        literal,
        func_symbol,
        method,
        struct_symbol,
        enum_symbol
    };

public:
    kind value_kind;
    std::string content;
    type resolve_type;

public:
    auto to_value_t() const {
        if (value_kind==mir_value_t::kind::literal ||
            value_kind==mir_value_t::kind::nil) {
            return value_t::literal(content);
        }
        if (value_kind==mir_value_t::kind::variable) {
            return value_t::variable(content);
        }
        return value_t::null();
    }

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
    static auto method(const std::string& full_name, const type& ty) {
        return mir_value_t {
            .value_kind = mir_value_t::kind::method,
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
    std::unordered_map<std::string, sym_kind> type_mapper;

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
    std::string type_mapping(const type&);

private:
    sir_context ictx;
    ssa_generator ssa_gen;
    sir_block* block;
    std::vector<mir_value_t> value_stack;
    error err;

    local_table locals;

    std::vector<usize> loop_entry;
    std::vector<std::vector<sir_br*>> break_inst;
    std::vector<std::vector<sir_br*>> branch_jump_out;

private:
    void unimplemented(mir* node) {
        std::stringstream ss;
        node->dump("", ss);
        err.err("mir2sir",
            node->get_location(),
            "internal error: unimplemented mir: " + ss.str()
        );
    }

private:
    void generate_type_mapper();
    void emit_struct(const mir_context&);
    void emit_func_decl(const mir_context&);
    void emit_func_impl(const mir_context&);

private:
    void generate_and(mir_binary*);
    void generate_or(mir_binary*);

private:
    void visit_mir_block(mir_block*) override;
    void visit_mir_unary(mir_unary*) override;
    void visit_mir_binary(mir_binary*) override;
    void visit_mir_type_convert(mir_type_convert*) override;
    void visit_mir_nil(mir_nil*) override;
    void visit_mir_number(mir_number*) override;
    void visit_mir_string(mir_string*) override;
    void visit_mir_char(mir_char*) override;
    void visit_mir_bool(mir_bool*) override;
    void call_expression_generation(mir_call*, bool);
    void visit_mir_call(mir_call*) override;
    void visit_mir_call_id(mir_call_id*) override;
    void visit_mir_call_index(mir_call_index*) override;
    void visit_mir_call_func(mir_call_func*) override;
    void visit_mir_get_field(mir_get_field*) override;
    void visit_mir_get_path(mir_get_path*) override;
    void visit_mir_ptr_get_field(mir_ptr_get_field*) override;
    void visit_mir_define(mir_define*) override;
    void visit_mir_assign(mir_assign*) override;
    void visit_mir_if(mir_if*) override;
    void visit_mir_branch(mir_branch*) override;
    void visit_mir_break(mir_break*) override;
    void visit_mir_continue(mir_continue*) override;
    void visit_mir_while(mir_while*) override;
    void visit_mir_return(mir_return*) override;

public:
    mir2sir(const semantic_context& c):
        ctx(c), block(nullptr) {}
    const error& generate(const mir_context&);
    auto& get_mutable_sir_context() { return ictx; }
};

}
