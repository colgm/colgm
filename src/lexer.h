#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "colgm.h"
#include "err.h"

namespace colgm {

enum class tok:u32 {
    null=0,   // null token (default token type)
    num,      // number literal
    str,      // string literal
    id,       // identifier
    tktrue,   // keyword true
    tkfalse,  // keyword false
    use,      // keyword use
    rfor,     // loop keyword for
    forindex, // loop keyword forindex
    foreach,  // loop keyword foreach
    rwhile,   // loop keyword while
    var,      // keyword for definition
    pub,      // keyword pub
    func,     // keyword for definition of function
    brk,      // loop keyword break
    cont,     // loop keyword continue
    ret,      // function keyword return
    rif,      // condition expression keyword if
    elsif,    // condition expression keyword elsif
    relse,    // condition expression keyword else
    tknil,    // nil literal
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
    mod,      // operator %
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
    modeq,    // operator %=
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

    const std::unordered_map<std::string, tok> typetbl {
        {"use"     ,tok::use     },
        {"true"    ,tok::tktrue  },
        {"false"   ,tok::tkfalse },
        {"for"     ,tok::rfor    },
        {"forindex",tok::forindex},
        {"foreach" ,tok::foreach },
        {"while"   ,tok::rwhile  },
        {"var"     ,tok::var     },
        {"pub"     ,tok::pub     },
        {"func"    ,tok::func    },
        {"break"   ,tok::brk     },
        {"continue",tok::cont    },
        {"return"  ,tok::ret     },
        {"if"      ,tok::rif     },
        {"elsif"   ,tok::elsif   },
        {"else"    ,tok::relse   },
        {"nil"     ,tok::tknil   },
        {"("       ,tok::lcurve  },
        {")"       ,tok::rcurve  },
        {"["       ,tok::lbracket},
        {"]"       ,tok::rbracket},
        {"{"       ,tok::lbrace  },
        {"}"       ,tok::rbrace  },
        {";"       ,tok::semi    },
        {"and"     ,tok::opand   },
        {"or"      ,tok::opor    },
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
        {"%"       ,tok::mod     },
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
        {"%="      ,tok::modeq   },
        {"~="      ,tok::lnkeq   },
        {"&="      ,tok::btandeq },
        {"|="      ,tok::btoreq  },
        {"^="      ,tok::btxoreq },
        {"=="      ,tok::cmpeq   },
        {"!="      ,tok::neq     },
        {"<"       ,tok::less    },
        {"<="      ,tok::leq     },
        {">"       ,tok::grt     },
        {">="      ,tok::geq     }
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

    void skip_note();
    void err_char();

    void open(const std::string&);
    std::string utf8_gen();
    token id_gen();
    token num_gen();
    token str_gen();
    token single_opr();
    token dots();
    token colons();
    token calc_opr();
public:
    lexer(): line(1), column(0), ptr(0), filename(""), res(""), invalid_char(0) {}
    const error& scan(const std::string&);
    const std::vector<token>& result() const {return toks;}
};

}
