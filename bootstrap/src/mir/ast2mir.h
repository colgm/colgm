#pragma once

#include "ast/visitor.h"
#include "sema/context.h"
#include "mir/mir.h"
#include "report.h"

#include <iostream>
#include <cstring>
#include <sstream>
#include <unordered_map>

namespace colgm::mir {

struct mir_func {
    std::string name;
    span location;
    std::vector<std::pair<std::string, type>> params;
    type return_type;
    mir_block* block;

    mir_func(): block(nullptr) {}
    ~mir_func() { delete block; }
};

class ast2mir: public visitor {
private:
    error err;
    const semantic_context& ctx;

private:
    std::unordered_map<std::string, mir_func*> impls;
    std::string impl_struct_name = "";
    mir_block* block = nullptr;

private:
    bool visit_unary_operator(unary_operator*) override;
    bool visit_binary_operator(binary_operator*) override;
    bool visit_nil_literal(nil_literal*) override;
    bool visit_number_literal(number_literal*) override;
    bool visit_string_literal(string_literal*) override;
    bool visit_char_literal(char_literal*) override;
    bool visit_bool_literal(bool_literal*) override;
    bool visit_func_decl(func_decl*) override;
    bool visit_impl_struct(impl_struct*) override;
    bool visit_call_index(call_index*) override;
    bool visit_call_func_args(call_func_args*) override;
    bool visit_call_field(call_field*) override;
    bool visit_ptr_call_field(ptr_call_field*) override;
    bool visit_call_path(call_path*) override;
    bool visit_call(call*) override;
    bool visit_definition(definition*) override;
    bool visit_code_block(code_block*) override;

private:
    type generate_type(type_def*);
    void emit_function(mir_func*);

public:
    ast2mir(const semantic_context& c): ctx(c) {}
    ~ast2mir() {
        for(const auto& i : impls) {
            delete i.second;
        }
    }

public:
    void dump(std::ostream&) const;
    auto& generate(root* ast_root) {
        ast_root->accept(this);
        dump(std::cout);
        return err;
    }
    auto& get_mutable_ir() const { return impls; }
};

}