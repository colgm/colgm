#pragma once

#include "colgm.h"
#include "report.h"
#include "lexer.h"
#include "ast/ast.h"
#include "ast/decl.h"
#include "ast/expr.h"
#include "ast/stmt.h"

#include <vector>
#include <unordered_map>

namespace colgm {

using namespace ast;

class generic_list_look_ahead {
private:
    usize ptr;
    const token* toks;

private:
    bool look_ahead(tok t) { return t == toks[ptr].type; }
    void next() {
        if (toks[ptr].type!=tok::tk_eof) {
            ++ptr;
        }
    }
    bool look_ahead_type_def();

public:
    generic_list_look_ahead(const token* t, usize p): ptr(p), toks(t) {}
    bool check();
};

class parse {
private:
    usize ptr;
    error& err;
    root* result;
    const token* toks;

    const std::unordered_map<tok, std::string> tokname = {
        {tok::tk_use         , "use"     },
        {tok::tk_enum        , "enum"    },
        {tok::tk_union       , "union"   },
        {tok::tk_impl        , "impl"    },
        {tok::tk_true        , "true"    },
        {tok::tk_false       , "false"   },
        {tok::tk_stct        , "struct"  },
        {tok::tk_for         , "for"     },
        {tok::tk_forindex    , "forindex"},
        {tok::tk_foreach     , "foreach" },
        {tok::tk_while       , "while"   },
        {tok::tk_var         , "var"     },
        {tok::tk_pub         , "pub"     },
        {tok::tk_extern      , "extern"  },
        {tok::tk_func        , "func"    },
        {tok::tk_match       , "match"   },
        {tok::tk_const       , "const"   },
        {tok::tk_brk         , "break"   },
        {tok::tk_cont        , "continue"},
        {tok::tk_ret         , "return"  },
        {tok::tk_if          , "if"      },
        {tok::tk_elsif       , "elsif"   },
        {tok::tk_else        , "else"    },
        {tok::tk_nil         , "nil"     },
        {tok::tk_lcurve      , "("       },
        {tok::tk_rcurve      , ")"       },
        {tok::tk_lbracket    , "["       },
        {tok::tk_rbracket    , "]"       },
        {tok::tk_lbrace      , "{"       },
        {tok::tk_rbrace      , "}"       },
        {tok::tk_semi        , ";"       },
        {tok::tk_opand       , "and"     },
        {tok::tk_opor        , "or"      },
        {tok::tk_comma       , ","       },
        {tok::tk_dot         , "."       },
        {tok::tk_ellipsis    , "..."     },
        {tok::tk_quesmark    , "?"       },
        {tok::tk_colon       , ":"       },
        {tok::tk_double_colon, "::"      },
        {tok::tk_add         , "+"       },
        {tok::tk_sub         , "-"       },
        {tok::tk_mult        , "*"       },
        {tok::tk_div         , "/"       },
        {tok::tk_rem         , "%"       },
        {tok::tk_floater     , "~"       },
        {tok::tk_btand       , "&"       },
        {tok::tk_btor        , "|"       },
        {tok::tk_btxor       , "^"       },
        {tok::tk_opnot       , "!"       },
        {tok::tk_eq          , "="       },
        {tok::tk_addeq       , "+="      },
        {tok::tk_subeq       , "-="      },
        {tok::tk_multeq      , "*="      },
        {tok::tk_diveq       , "/="      },
        {tok::tk_remeq       , "&="      },
        {tok::tk_lnkeq       , "~="      },
        {tok::tk_btandeq     , "&="      },
        {tok::tk_btoreq      , "|="      },
        {tok::tk_btxoreq     , "^="      },
        {tok::tk_cmpeq       , "=="      },
        {tok::tk_neq         , "!="      },
        {tok::tk_less        , "<"       },
        {tok::tk_leq         , "<="      },
        {tok::tk_grt         , ">"       },
        {tok::tk_geq         , ">="      },
        {tok::tk_arrow       , "->"      },
        {tok::tk_wide_arrow  , "=>"      },
        {tok::tk_sharp       , "#"       }
    };

private:
    void try_match_semi();
    void try_match_rbrace();
    void match(tok);
    void next() {
        if (toks[ptr].type!=tok::tk_eof) {
            ++ptr;
        }
    }
    bool look_ahead(tok t) { return t == toks[ptr].type; }
    bool look_ahead_generic() {
        return generic_list_look_ahead(toks, ptr).check();
    }
    void update_location(node* n) {
        n->update_location(toks[ptr - 1].loc);
    }

private:
    cond_compile* conditional_compile();
    identifier* identifier_gen();
    call* call_gen();
    call_id* call_id_gen();
    initializer* initializer_gen();
    nil_literal* nil_gen();
    number_literal* number_gen();
    string_literal* string_gen();
    char_literal* char_gen();
    bool_literal* bool_gen();
    array_list* array_list_gen();
    expr* scalar_gen();
    unary_operator* unary_neg_gen();
    unary_operator* unary_bnot_gen();
    expr* type_convert_gen();
    expr* multive_gen();
    expr* additive_gen();
    expr* bitwise_and_gen();
    expr* bitwise_xor_gen();
    expr* bitwise_or_gen();
    expr* compare_gen();
    expr* not_expression_gen();
    expr* and_expression_gen();
    expr* or_expression_gen();
    expr* calculation_gen();
    type_def* array_type_gen();
    type_def* type_def_gen();
    generic_type_list* generic_type_list_gen();
    enum_decl* enum_gen(std::vector<cond_compile*>&, bool, bool);
    field_pair* field_pair_gen();
    struct_decl* struct_gen(std::vector<cond_compile*>&, bool, bool);
    tagged_union_decl* tagged_union_gen(std::vector<cond_compile*>&, bool, bool);
    param* param_gen();
    param_list* param_list_gen();
    func_decl* function_gen(std::vector<cond_compile*>&, bool, bool);
    impl_struct* impl_gen(std::vector<cond_compile*>&, bool, bool);
    use_stmt* use_stmt_gen();
    definition* definition_gen();
    cond_stmt* cond_stmt_gen();
    match_stmt* match_stmt_gen();
    while_stmt* while_stmt_gen();
    for_stmt* for_stmt_gen();
    forindex* forindex_gen();
    foreach* foreach_gen();
    void add_gen_stmt(code_block*);
    code_block* block_gen(bool);
    in_stmt_expr* in_stmt_expr_gen();
    ret_stmt* return_gen();
    continue_stmt* continue_gen();
    break_stmt* break_gen();

public:
    parse(error& e): ptr(0), err(e), result(nullptr), toks(nullptr) {}
    ~parse() { delete result; }
    const error& analyse(const std::vector<token>&);
    auto get_result() { return result; }
};
    
}
