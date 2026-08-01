#pragma once

#include <string>

#include "colgm.h"
#include "error/report.h"

namespace colgm {

enum class tok : u32 {
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

}
