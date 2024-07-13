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

class parse {
private:
    usize ptr;
    error err;
    root* result;
    const token* toks;

    const std::unordered_map<tok, std::string> tokname = {
        {tok::tk_use         , "use"     },
        {tok::tk_enum        , "enum"    },
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
        {tok::tk_func        , "func"    },
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
        {tok::tk_double_colon, "::"     },
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
        {tok::tk_wide_arrow  , "=>"      }
    };

private:
    void match(tok);
    void next() {
        if (toks[ptr].type!=tok::tk_eof) {
            ++ptr;
        }
    }
    bool look_ahead(tok);
    void update_location(node*);

private:
    identifier* identifier_gen();
    call* call_gen();
    nil_literal* nil_gen();
    number_literal* number_gen();
    string_literal* string_gen();
    char_literal* char_gen();
    bool_literal* bool_gen();
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
    expr* and_expression_gen();
    expr* or_expression_gen();
    expr* calculation_gen();
    type_def* type_gen();
    struct_field* struct_field_gen();
    struct_decl* struct_gen();
    param* param_gen();
    param_list* param_list_gen();
    func_decl* function_gen();
    impl_struct* impl_gen();
    use_stmt* use_stmt_gen();
    definition* definition_gen();
    cond_stmt* cond_stmt_gen();
    while_stmt* while_stmt_gen();
    code_block* block_gen();
    in_stmt_expr* in_stmt_expr_gen();
    ret_stmt* return_gen();
    continue_stmt* continue_gen();
    break_stmt* break_gen();

public:
    const error& analyse(const std::vector<token>&);
    auto get_result() { return result; }
};
    
}
