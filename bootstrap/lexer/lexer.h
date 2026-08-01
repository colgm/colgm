#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "colgm.h"
#include "error/report.h"
#include "lexer/token.h"

namespace colgm {

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
