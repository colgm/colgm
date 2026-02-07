#pragma once

#include "report.h"
#include "sema/type.h"
#include "sema/context.h"
#include "mir/visitor.h"
#include "mir/ast2mir.h"
#include "sir/sir.h"
#include "sir/debug_info.h"
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
    auto create_index() { counter++; return counter - 1; }
    auto create() { return std::to_string(counter++); }
};

struct local_table {
    std::vector<std::unordered_map<std::string, std::string>> elem;


    auto get_local(const std::string& name) {
        for (auto it = elem.rbegin(); it != elem.rend(); ++it) {
            if (it->count(name)) {
                return it->at(name);
            }
        }
        return "<" + name + "-not-found>";
    }

    auto size() const { return elem.size(); }

    void push() { elem.push_back({}); }
    void pop() { elem.pop_back(); }
};

struct mir_value_t {
public:
    enum class kind {
        null, // reserved
        nil,
        variable,
        literal,
        primitive,
        func_symbol,
        method,
        struct_symbol,
        tagged_union_symbol,
        enum_symbol
    };

public:
    kind value_kind;
    std::string content;
    type resolve_type;

public:
    auto to_value_t() const {
        switch (value_kind) {
            case mir_value_t::kind::null: return value_t::null("null");
            case mir_value_t::kind::nil:
            case mir_value_t::kind::literal: return value_t::literal(content);
            case mir_value_t::kind::variable: return value_t::variable(content);
            case mir_value_t::kind::primitive: return value_t::null("primitive");
            case mir_value_t::kind::func_symbol: return value_t::null("func");
            case mir_value_t::kind::method: return value_t::null("method");
            case mir_value_t::kind::struct_symbol: return value_t::null("struct");
            case mir_value_t::kind::tagged_union_symbol: return value_t::null("tagged_union");
            case mir_value_t::kind::enum_symbol: return value_t::null("enum");
        }
        // unreachable
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
    static auto primitive(const std::string& value, const type& ty) {
        return mir_value_t {
            .value_kind = mir_value_t::kind::primitive,
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
    static auto tagged_union_kind(const std::string& full_name, const type& ty) {
        return mir_value_t {
            .value_kind = mir_value_t::kind::tagged_union_symbol,
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

struct DWARF_status {
    u64 DI_counter;
    u64 compile_unit_index;
    u64 scope_index;
    std::unordered_map<std::string, u64> impl_debug_info;
};

class mir2sir: public visitor {
private:
    const sema_context& ctx;
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
    std::string array_type_mapping(const type&);

private:
    sir_context ictx;
    ssa_generator ssa_gen;
    ssa_generator array_ssa_gen;
    ssa_generator var_ssa_gen;
    ssa_generator label_gen;

    sir_block* func_block;
    sir_basic_block* alloca_block;
    sir_basic_block* move_reg_block;
    sir_basic_block* block;

    std::vector<mir_value_t> value_stack;
    error err;

    local_table locals;

    std::vector<std::vector<sir_br*>> continue_inst;
    std::vector<std::vector<sir_br*>> break_inst;
    std::vector<std::vector<sir_br*>> branch_jump_out;

    DWARF_status dwarf_status;

    std::unordered_map<std::string, mir_struct*> struct_mapper;
    std::unordered_map<std::string, mir_tagged_union*> tagged_union_mapper;

private:
    void unimplemented(mir* node) {
        std::stringstream ss;
        node->dump("", ss);
        err.err(node->get_location(),
            "internal error: unimplemented mir: " + ss.str()
        );
    }

private:
    void generate_type_mapper();
    void emit_tagged_union(const mir_context&);
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
    void visit_mir_array(mir_array*) override;
    void de_reference();
    void call_expression_generation(mir_call*, bool);
    void visit_mir_call(mir_call*) override;
    void visit_mir_struct_init(mir_struct_init*) override;
    void push_global_func(const type&);
    void visit_mir_call_id(mir_call_id*) override;
    void visit_mir_call_index(mir_call_index*) override;
    void visit_mir_call_func(mir_call_func*) override;
    void visit_mir_get_field(mir_get_field*) override;
    void visit_mir_get_path(mir_get_path*) override;
    void visit_mir_ptr_get_field(mir_ptr_get_field*) override;
    void visit_mir_define(mir_define*) override;
    void visit_mir_assign(mir_assign*) override;
    void mir_assign_gen(const mir_value_t&, const mir_value_t&, mir_assign*);
    void visit_mir_if(mir_if*) override;
    void visit_mir_branch(mir_branch*) override;
    void visit_mir_switch_case(mir_switch_case*) override;
    void visit_mir_switch(mir_switch*) override;
    void visit_mir_break(mir_break*) override;
    void visit_mir_continue(mir_continue*) override;
    void visit_mir_loop(mir_loop*) override;
    void visit_mir_return(mir_return*) override;

private:
    void generate_llvm_ident();
    void generate_llvm_module_flags();
    void generate_llvm_dbg_cu();
    void generate_DIFile();
    void generate_basic_type();
    void generate_DI_enum_type();
    void generate_DI_structure_type(const mir_context&);
    void generate_DI_subprogram(const mir_context&);
    u64 generate_DI_location(const span&);
    void generate_DWARF(const mir_context&);

public:
    struct size_align_pair {
        u64 size;
        u64 align;
    };

private:
    size_align_pair calculate_size_and_align(const type&);
    void calculate_single_tagged_union_size(mir_tagged_union*);
    void calculate_single_struct_size(mir_struct*);
    void calculate_size(const mir_context&);

public:
    mir2sir(const sema_context& c):
        ctx(c), func_block(nullptr),
        alloca_block(nullptr), move_reg_block(nullptr), block(nullptr) {}
    const error& generate(const mir_context&);
    auto& get_mutable_sir_context() { return ictx; }
};

}
