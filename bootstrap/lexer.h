#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "colgm.h"
#include "report.h"

namespace colgm {

enum class tok:u32 {
    tk_null = 0,     // null token (default token type)
    tk_num,          // number literal
    tk_str,          // string literal
    tk_ch,           // char literal
    tk_id,           // identifier
    tk_true,         // keyword true
    tk_false,        // keyword false
    tk_use,          // keyword use
    tk_enum,         // keyword enum
    tk_union,        // keyword union
    tk_for,          // loop keyword for
    tk_forindex,     // loop keyword forindex
    tk_foreach,      // loop keyword foreach
    tk_while,        // loop keyword while
    tk_var,          // keyword for definition
    tk_stct,         // keyword for struct
    tk_pub,          // keyword pub
    tk_extern,       // keyword extern
    tk_impl,         // keyword impl
    tk_func,         // keyword for definition of function
    tk_match,        // keyword for match
    tk_const,        // keyword for constant
    tk_defer,        // keyword defer
    tk_brk,          // loop keyword break
    tk_cont,         // loop keyword continue
    tk_ret,          // function keyword return
    tk_if,           // condition expression keyword if
    tk_elsif,        // condition expression keyword elsif
    tk_else,         // condition expression keyword else
    tk_nil,          // nil literal
    tk_lcurve,       // (
    tk_rcurve,       // )
    tk_lbracket,     // [
    tk_rbracket,     // ]
    tk_lbrace,       // {
    tk_rbrace,       // }
    tk_semi,         // ;
    tk_opand,        // operator and
    tk_opor,         // operator or
    tk_comma,        // ,
    tk_dot,          // .
    tk_ellipsis,     // ...
    tk_quesmark,     // ?
    tk_colon,        // :
    tk_double_colon, // ::
    tk_add,          // operator +
    tk_sub,          // operator -
    tk_mult,         // operator *
    tk_div,          // operator /
    tk_rem,          // operator %
    tk_floater,      // operator ~ and binary operator ~
    tk_btand,        // bitwise operator &
    tk_btor,         // bitwise operator |
    tk_btxor,        // bitwise operator ^
    tk_opnot,        // operator !
    tk_eq,           // operator =
    tk_addeq,        // operator +=
    tk_subeq,        // operator -=
    tk_multeq,       // operator *=
    tk_diveq,        // operator /=
    tk_remeq,        // operator %=
    tk_lnkeq,        // operator ~=
    tk_btandeq,      // operator &=
    tk_btoreq,       // operator |=
    tk_btxoreq,      // operator ^=
    tk_cmpeq,        // operator ==
    tk_neq,          // operator !=
    tk_less,         // operator <
    tk_leq,          // operator <=
    tk_grt,          // operator >
    tk_geq,          // operator >=
    tk_arrow,        // operator ->
    tk_wide_arrow,   // operator =>
    tk_sharp,        // #
    tk_eof           // <eof> end of token list
};

struct token {
    span loc; // location
    tok type; // token type
    std::string str; // content
    token() = default;
    token(const token&) = default;
    token(const span& loc, tok type, const std::string& str):
        loc(loc), type(type), str(str) {}
};

class lexer {
private:
    u32 line;
    u32 column;
    usize ptr;
    std::string filename;
    std::string res;

    error& err;
    u64 invalid_char;
    std::vector<token> toks;

    const std::unordered_map<std::string, tok> type_table = {
        {"use"     , tok::tk_use         },
        {"enum"    , tok::tk_enum        },
        {"union"   , tok::tk_union       },
        {"impl"    , tok::tk_impl        },
        {"true"    , tok::tk_true        },
        {"false"   , tok::tk_false       },
        {"for"     , tok::tk_for         },
        {"forindex", tok::tk_forindex    },
        {"foreach" , tok::tk_foreach     },
        {"while"   , tok::tk_while       },
        {"var"     , tok::tk_var         },
        {"struct"  , tok::tk_stct        },
        {"pub"     , tok::tk_pub         },
        {"extern"  , tok::tk_extern      },
        {"func"    , tok::tk_func        },
        {"match"   , tok::tk_match       },
        {"const"   , tok::tk_const       },
        {"defer"   , tok::tk_defer       },
        {"break"   , tok::tk_brk         },
        {"continue", tok::tk_cont        },
        {"return"  , tok::tk_ret         },
        {"if"      , tok::tk_if          },
        {"elsif"   , tok::tk_elsif       },
        {"else"    , tok::tk_else        },
        {"nil"     , tok::tk_nil         },
        {"("       , tok::tk_lcurve      },
        {")"       , tok::tk_rcurve      },
        {"["       , tok::tk_lbracket    },
        {"]"       , tok::tk_rbracket    },
        {"{"       , tok::tk_lbrace      },
        {"}"       , tok::tk_rbrace      },
        {";"       , tok::tk_semi        },
        {"and"     , tok::tk_opand       },
        {"&&"      , tok::tk_opand       },
        {"or"      , tok::tk_opor        },
        {"||"      , tok::tk_opor        },
        {","       , tok::tk_comma       },
        {"."       , tok::tk_dot         },
        {"..."     , tok::tk_ellipsis    },
        {"?"       , tok::tk_quesmark    },
        {":"       , tok::tk_colon       },
        {"::"      , tok::tk_double_colon},
        {"+"       , tok::tk_add         },
        {"-"       , tok::tk_sub         },
        {"*"       , tok::tk_mult        },
        {"/"       , tok::tk_div         },
        {"%"       , tok::tk_rem         },
        {"~"       , tok::tk_floater     },
        {"&"       , tok::tk_btand       },
        {"|"       , tok::tk_btor        },
        {"^"       , tok::tk_btxor       },
        {"!"       , tok::tk_opnot       },
        {"="       , tok::tk_eq          },
        {"+="      , tok::tk_addeq       },
        {"-="      , tok::tk_subeq       },
        {"*="      , tok::tk_multeq      },
        {"/="      , tok::tk_diveq       },
        {"%="      , tok::tk_remeq       },
        {"~="      , tok::tk_lnkeq       },
        {"&="      , tok::tk_btandeq     },
        {"|="      , tok::tk_btoreq      },
        {"^="      , tok::tk_btxoreq     },
        {"=="      , tok::tk_cmpeq       },
        {"!="      , tok::tk_neq         },
        {"<"       , tok::tk_less        },
        {"<="      , tok::tk_leq         },
        {">"       , tok::tk_grt         },
        {">="      , tok::tk_geq         },
        {"->"      , tok::tk_arrow       },
        {"=>"      , tok::tk_wide_arrow  },
        {"#"       , tok::tk_sharp       }
    };

    tok get_type(const std::string&);
    bool skip(char);
    bool is_id(char);
    bool is_hex(char);
    bool is_oct(char);
    bool is_dec(char);
    bool is_str(char);
    bool is_single_opr(char);
    bool is_calc_opr(char);
    bool is_arrow(char);
    bool is_wide_arrow(char);
    bool is_note();
    bool is_multi_line_comment();

    void skip_note();
    void skip_multi_line_comment();
    void err_char();

    void open(const std::string&);
    std::string utf8_gen();
    token id_gen();
    token num_gen();
    token str_gen();
    token arrow_gen();
    token single_opr();
    token dots();
    token colons();
    token calc_opr();

public:
    lexer(error& e):
        line(1), column(0), ptr(0),
        filename(""), res(""), err(e), invalid_char(0) {}
    const error& scan(const std::string&);
    const auto& result() const { return toks; }
};

}
