#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "colgm.h"
#include "report.h"

namespace colgm {

enum class tok:u32 {
    null = 0, // null token (default token type)
    num,      // number literal
    str,      // string literal
    ch,       // char literal
    id,       // identifier
    tk_true,  // keyword true
    tk_false, // keyword false
    use,      // keyword use
    tk_enum,  // keyword enum
    rfor,     // loop keyword for
    forindex, // loop keyword forindex
    foreach,  // loop keyword foreach
    rwhile,   // loop keyword while
    var,      // keyword for definition
    stct,     // keyword for struct
    pub,      // keyword pub
    impl,     // keyword impl
    func,     // keyword for definition of function
    brk,      // loop keyword break
    cont,     // loop keyword continue
    ret,      // function keyword return
    rif,      // condition expression keyword if
    elsif,    // condition expression keyword elsif
    relse,    // condition expression keyword else
    nil,      // nil literal
    lcurve,   // (
    rcurve,   // )
    lbracket, // [
    rbracket, // ]
    lbrace,   // {
    rbrace,   // }
    semi,     // ;
    opand,    // operator and
    opor,     // operator or
    comma,    // ,
    dot,      // .
    ellipsis, // ...
    quesmark, // ?
    colon,    // :
    double_colon, // ::
    add,      // operator +
    sub,      // operator -
    mult,     // operator *
    div,      // operator /
    rem,      // operator %
    floater,  // operator ~ and binary operator ~
    btand,    // bitwise operator &
    btor,     // bitwise operator |
    btxor,    // bitwise operator ^
    opnot,    // operator !
    eq,       // operator =
    addeq,    // operator +=
    subeq,    // operator -=
    multeq,   // operator *=
    diveq,    // operator /=
    remeq,    // operator %=
    lnkeq,    // operator ~=
    btandeq,  // operator &=
    btoreq,   // operator |=
    btxoreq,  // operator ^=
    cmpeq,    // operator ==
    neq,      // operator !=
    less,     // operator <
    leq,      // operator <=
    grt,      // operator >
    geq,      // operator >=
    arrow,    // operator ->
    eof       // <eof> end of token list
};

struct token {
    span loc; // location
    tok type; // token type
    std::string str; // content
    token() = default;
    token(const token&) = default;
};

class lexer {
private:
    u32 line;
    u32 column;
    usize ptr;
    std::string filename;
    std::string res;

    error err;
    u64 invalid_char;
    std::vector<token> toks;

    const std::unordered_map<std::string, tok> type_table = {
        {"use"     ,tok::use     },
        {"enum"    ,tok::tk_enum },
        {"impl"    ,tok::impl    },
        {"true"    ,tok::tk_true },
        {"false"   ,tok::tk_false},
        {"for"     ,tok::rfor    },
        {"forindex",tok::forindex},
        {"foreach" ,tok::foreach },
        {"while"   ,tok::rwhile  },
        {"var"     ,tok::var     },
        {"struct"  ,tok::stct    },
        {"pub"     ,tok::pub     },
        {"func"    ,tok::func    },
        {"break"   ,tok::brk     },
        {"continue",tok::cont    },
        {"return"  ,tok::ret     },
        {"if"      ,tok::rif     },
        {"elsif"   ,tok::elsif   },
        {"else"    ,tok::relse   },
        {"nil"     ,tok::nil     },
        {"("       ,tok::lcurve  },
        {")"       ,tok::rcurve  },
        {"["       ,tok::lbracket},
        {"]"       ,tok::rbracket},
        {"{"       ,tok::lbrace  },
        {"}"       ,tok::rbrace  },
        {";"       ,tok::semi    },
        {"and"     ,tok::opand   },
        {"&&"      ,tok::opand   },
        {"or"      ,tok::opor    },
        {"||"      ,tok::opor    },
        {","       ,tok::comma   },
        {"."       ,tok::dot     },
        {"..."     ,tok::ellipsis},
        {"?"       ,tok::quesmark},
        {":"       ,tok::colon   },
        {"::"      ,tok::double_colon},
        {"+"       ,tok::add     },
        {"-"       ,tok::sub     },
        {"*"       ,tok::mult    },
        {"/"       ,tok::div     },
        {"%"       ,tok::rem     },
        {"~"       ,tok::floater },
        {"&"       ,tok::btand   },
        {"|"       ,tok::btor    },
        {"^"       ,tok::btxor   },
        {"!"       ,tok::opnot   },
        {"="       ,tok::eq      },
        {"+="      ,tok::addeq   },
        {"-="      ,tok::subeq   },
        {"*="      ,tok::multeq  },
        {"/="      ,tok::diveq   },
        {"%="      ,tok::remeq   },
        {"~="      ,tok::lnkeq   },
        {"&="      ,tok::btandeq },
        {"|="      ,tok::btoreq  },
        {"^="      ,tok::btxoreq },
        {"=="      ,tok::cmpeq   },
        {"!="      ,tok::neq     },
        {"<"       ,tok::less    },
        {"<="      ,tok::leq     },
        {">"       ,tok::grt     },
        {">="      ,tok::geq     },
        {"->"      ,tok::arrow   }
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

    void skip_note();
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
    lexer(): line(1), column(0), ptr(0), filename(""), res(""), invalid_char(0) {}
    const error& scan(const std::string&);
    const auto& result() const { return toks; }
};

}
