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
        {tok::use     ,"use"     },
        {tok::impl    ,"impl"    },
        {tok::tktrue  ,"true"    },
        {tok::tkfalse ,"false"   },
        {tok::stct    ,"struct"  },
        {tok::rfor    ,"for"     },
        {tok::forindex,"forindex"},
        {tok::foreach ,"foreach" },
        {tok::rwhile  ,"while"   },
        {tok::var     ,"var"     },
        {tok::pub     ,"pub"     },
        {tok::func    ,"func"    },
        {tok::brk     ,"break"   },
        {tok::cont    ,"continue"},
        {tok::ret     ,"return"  },
        {tok::rif     ,"if"      },
        {tok::elsif   ,"elsif"   },
        {tok::relse   ,"else"    },
        {tok::tknil   ,"nil"     },
        {tok::lcurve  ,"("       },
        {tok::rcurve  ,")"       },
        {tok::lbracket,"["       },
        {tok::rbracket,"]"       },
        {tok::lbrace  ,"{"       },
        {tok::rbrace  ,"}"       },
        {tok::semi    ,";"       },
        {tok::opand   ,"and"     },
        {tok::opor    ,"or"      },
        {tok::comma   ,","       },
        {tok::dot     ,"."       },
        {tok::ellipsis,"..."     },
        {tok::quesmark,"?"       },
        {tok::colon   ,":"       },
        {tok::double_colon, "::" },
        {tok::add     ,"+"       },
        {tok::sub     ,"-"       },
        {tok::mult    ,"*"       },
        {tok::div     ,"/"       },
        {tok::rem     ,"%"       },
        {tok::floater ,"~"       },
        {tok::btand   ,"&"       },
        {tok::btor    ,"|"       },
        {tok::btxor   ,"^"       },
        {tok::opnot   ,"!"       },
        {tok::eq      ,"="       },
        {tok::addeq   ,"+="      },
        {tok::subeq   ,"-="      },
        {tok::multeq  ,"*="      },
        {tok::diveq   ,"/="      },
        {tok::remeq   ,"&="      },
        {tok::lnkeq   ,"~="      },
        {tok::btandeq ,"&="      },
        {tok::btoreq  ,"|="      },
        {tok::btxoreq ,"^="      },
        {tok::cmpeq   ,"=="      },
        {tok::neq     ,"!="      },
        {tok::less    ,"<"       },
        {tok::leq     ,"<="      },
        {tok::grt     ,">"       },
        {tok::geq     ,">="      },
        {tok::arrow   ,"->"      }
    };

private:
    void match(tok);
    void next() {
        if (toks[ptr].type!=tok::eof) {
            ++ptr;
        }
    }
    bool look_ahead(tok);
    void update_location(node*);

private:
    identifier* identifier_gen();
    call* call_gen();
    number_literal* number_gen();
    string_literal* string_gen();
    char_literal* char_gen();
    bool_literal* bool_gen();
    expr* scalar_gen();
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

public:
    const error& analyse(const std::vector<token>&);
    auto get_result() { return result; }
};
    
}
