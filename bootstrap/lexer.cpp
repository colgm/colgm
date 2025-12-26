#include "lexer.h"

#include <filesystem>

namespace colgm {

bool lexer::skip(char c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == 0;
}

bool lexer::is_id(char c) {
    return (c == '_') || std::isalpha(c) || (c < 0);
}

bool lexer::is_hex(char c) {
    return std::isxdigit(c);
}

bool lexer::is_oct(char c) {
    return '0' <= c && c <= '7';
}

bool lexer::is_dec(char c) {
    return std::isdigit(c);
}

bool lexer::is_str(char c) {
    return c == '\'' || c == '\"';
}

bool lexer::is_single_opr(char c) {
    return (
        c == '(' || c == ')' || c == '[' || c == ']' ||
        c == '{' || c == '}' || c == ',' || c == ';' ||
        c == '?' || c == '`' || c == '@' || c == '$' ||
        c == '\\' || c == '#'
    );
}

bool lexer::is_calc_opr(char c) {
    return (
        c == '=' || c == '+' || c == '-' || c == '*' ||
        c == '!' || c == '/' || c == '<' || c == '>' ||
        c == '~' || c == '|' || c == '&' || c == '^' ||
        c == '%'
    );
}

bool lexer::is_arrow(char c) {
    if (c != '-') {
        return false;
    }
    if (ptr + 1 < res.size() && res[ptr + 1] == '>') {
        return true;
    }
    return false;
}

bool lexer::is_wide_arrow(char c) {
    if (c != '=') {
        return false;
    }
    if (ptr + 1 < res.size() && res[ptr + 1] == '>') {
        return true;
    }
    return false;
}

bool lexer::is_note() {
    return res[ptr] == '/' && ptr + 1 < res.size() && res[ptr+1] == '/';
}

bool lexer::is_multi_line_comment() {
    return res[ptr] == '/' && ptr + 1 < res.size() && res[ptr+1] == '*';
}

void lexer::skip_note() {
    // avoid note, after this process ptr will point to '\n'
    // so next loop line counter+1
    while (++ptr < res.size() && res[ptr] != '\n') {}
}

void lexer::skip_multi_line_comment() {
    ++ptr;
    ++column;
    while (++ptr < res.size()) {
        ++column;
        if (res[ptr] == '\n') {
            ++line;
            column = 0;
        }
        if (res[ptr] == '*' && ptr + 1 < res.size() && res[ptr + 1] == '/') {
            ptr += 2;
            column += 2;
            return;
        }
    }

    err.err(
        {line, column, line, column, filename},
        "unclosed multi-line comment"
    );
}

void lexer::err_char() {
    ++column;
    char c = res[ptr++];
    err.err(
        {line, column-1, line, column, filename},
        "invalid character 0x" + char_to_hex(c)
    );
    ++invalid_char;
}

void lexer::open(const std::string& file) {
    // check file exsits and it is a regular file
    if (!std::filesystem::is_regular_file(file)) {
        err.err("<" + file + "> is not a regular file");
        err.chkerr();
    }

    // load
    filename = file;
    std::ifstream in(file, std::ios::binary);
    if (in.fail()) {
        err.err("failed to open <" + file + ">");
        res = "";
        return;
    }
    err.load(file);
    std::stringstream ss;
    ss << in.rdbuf();
    res = ss.str();
}

tok lexer::get_type(const std::string& str) {
    return type_table.count(str)? type_table.at(str):tok::tk_null;
}

std::string lexer::utf8_gen() {
    std::string str = "";
    while (ptr<res.size() && res[ptr]<0) {
        std::string tmp = "";
        u32 nbytes = utf8_hdchk(res[ptr]);
        if (!nbytes) {
            ++ptr;
            ++column;
            continue;
        }

        tmp += res[ptr++];
        for (u32 i = 0; i < nbytes; ++i, ++ptr) {
            if (ptr < res.size() && (res[ptr] & 0xc0) == 0x80) {
                tmp += res[ptr];
            }
        }

        // utf8 character's total length is 1+nbytes
        if (tmp.length() != 1 + nbytes) {
            ++column;
            std::string utf_info = "0x" + char_to_hex(tmp[0]);
            for (u32 i = 1; i < tmp.size(); ++i) {
                utf_info += " 0x" + char_to_hex(tmp[i]);
            }
            err.err(
                {line, column-1, line, column, filename},
                "invalid utf-8 <"+utf_info+">"
            );
            ++invalid_char;
        }
        str += tmp;
        // may have some problems because not all the unicode takes 2 space
        column += 2;
    }
    return str;
}

token lexer::id_gen() {
    u32 begin_line = line;
    u32 begin_column = column;
    std::string str = "";
    while (ptr<res.size() && (is_id(res[ptr]) || is_dec(res[ptr]))) {
        if (res[ptr]<0) { // utf-8
            str += utf8_gen();
        } else { // ascii
            str += res[ptr++];
            ++column;
        }
    }
    tok type = get_type(str);
    return token {
        {begin_line, begin_column, line, column, filename},
        (type != tok::tk_null) ? type : tok::tk_id,
        str
    };
}

token lexer::num_gen() {
    u32 begin_line = line;
    u32 begin_column = column;
    // generate hex number
    if (ptr + 1 < res.size() && res[ptr] == '0' && res[ptr + 1] == 'x') {
        std::string str = "0x";
        ptr += 2;
        while (ptr<res.size() && is_hex(res[ptr])) {
            str += res[ptr++];
        }
        column += str.length();
        // "0x"
        if (str.length()<3) {
            err.err(
                {begin_line, begin_column, line, column, filename},
                "invalid number `"+str+"`"
            );
        }
        return token {
            {begin_line, begin_column, line, column, filename},
            tok::tk_num,
            str
        };
    } else if (ptr + 1 < res.size() && res[ptr] == '0' && res[ptr + 1] == 'o') {
        // generate oct number
        std::string str = "0o";
        ptr += 2;
        while (ptr<res.size() && is_oct(res[ptr])) {
            str += res[ptr++];
        }
        bool erfmt = false;
        while (ptr<res.size() && (is_dec(res[ptr]) || is_hex(res[ptr]))) {
            erfmt = true;
            str += res[ptr++];
        }
        column += str.length();
        if (str.length() == 2 || erfmt) {
            err.err(
                {begin_line, begin_column, line, column, filename},
                "invalid number `"+str+"`"
            );
        }
        return token {
            {begin_line, begin_column, line, column, filename},
            tok::tk_num,
            str
        };
    }
    // generate dec number
    // dec number -> [0~9][0~9]*(.[0~9]*)(e|E(+|-)0|[1~9][0~9]*)
    std::string str = "";
    while (ptr < res.size() && is_dec(res[ptr])) {
        str += res[ptr++];
    }
    if (ptr < res.size() && res[ptr] == '.') {
        str += res[ptr++];
        while (ptr < res.size() && is_dec(res[ptr])) {
            str += res[ptr++];
        }
        // "xxxx." is not a correct number
        if (str.back() == '.') {
            column += str.length();
            err.err(
                {begin_line, begin_column, line, column, filename},
                "invalid number `" + str + "`"
            );
            return token {
                {begin_line, begin_column, line, column, filename},
                tok::tk_num,
                "0"
            };
        }
    }
    if (ptr < res.size() && (res[ptr] == 'e' || res[ptr] == 'E')) {
        str += res[ptr++];
        if (ptr < res.size() && (res[ptr] == '-' || res[ptr] == '+')) {
            str += res[ptr++];
        }
        while (ptr < res.size() && is_dec(res[ptr])) {
            str += res[ptr++];
        }
        // "xxxe(-|+)" is not a correct number
        if (str.back() == 'e' || str.back() == 'E' || str.back() == '-' || str.back() == '+') {
            column += str.length();
            err.err(
                {begin_line, begin_column, line, column, filename},
                "invalid number `" + str + "`"
            );
            return token {
                {begin_line, begin_column, line, column, filename},
                tok::tk_num,
                "0"
            };
        }
    }
    column += str.length();
    return token {
        {begin_line, begin_column, line, column, filename},
        tok::tk_num,
        str
    };
}

token lexer::str_gen() {
    u32 begin_line = line;
    u32 begin_column = column;
    std::string str = "";
    const char begin = res[ptr];
    ++column;
    while (++ptr < res.size() && res[ptr] != begin) {
        ++column;
        if (res[ptr] == '\n') {
            column = 0;
            ++line;
        }
        if (res[ptr] == '\\' && ptr + 1 < res.size()) {
            ++column;
            ++ptr;
            switch(res[ptr]) {
                case '0': str += '\0';    break;
                case 'a': str += '\a';    break;
                case 'b': str += '\b';    break;
                case 'e': str += '\033';  break;
                case 't': str += '\t';    break;
                case 'n': str += '\n';    break;
                case 'v': str += '\v';    break;
                case 'f': str += '\f';    break;
                case 'r': str += '\r';    break;
                case '?': str += '\?';    break;
                case '\\':str += '\\';    break;
                case '\'':str += '\'';    break;
                case '\"':str += '\"';    break;
                default:  str += res[ptr];break;
            }
            if (res[ptr] == '\n') {
                column = 0;
                ++line;
            }
            continue;
        }
        str += res[ptr];
    }
    // check if this string ends with a " or '
    if (ptr++ >= res.size()) {
        err.err(
            {begin_line, begin_column, line, column, filename},
            "get EOF when generating string"
        );
        return token {
            {begin_line, begin_column, line, column, filename},
            tok::tk_str,
            str
        };
    }
    ++column;

    // if is not utf8, 1+utf8_hdchk should be 1
    if (begin == '\'' && str.length() != 1 + utf8_hdchk(str[0])) {
        err.err(
            {begin_line, begin_column, line, column, filename},
            "\"\'\" is used for single character"
        );
    }
    // ascii character
    if (begin == '\'' && str.length() == 1) {
        return token {
            {begin_line, begin_column, line, column, filename},
            tok::tk_ch,
            str
        };
    }
    return token {
        {begin_line, begin_column, line, column, filename},
        tok::tk_str,
        str
    };
}

token lexer::arrow_gen() {
    u32 begin_line = line;
    u32 begin_column = column;
    std::string str = (res[ptr] == '-' ? "->" : "=>");
    ptr += str.length();
    column += str.length();
    return token {
        {begin_line, begin_column, line, column, filename},
        get_type(str),
        str
    };
}

token lexer::single_opr() {
    u32 begin_line = line;
    u32 begin_column = column;
    std::string str(1, res[ptr]);
    ++column;
    tok type = get_type(str);
    if (type == tok::tk_null) {
        err.err(
            {begin_line, begin_column, line, column, filename},
            "invalid operator `"+str+"`"
        );
    }
    ++ptr;
    return token {
        {begin_line, begin_column, line, column, filename},
        type,
        str
    };
}

token lexer::dots() {
    u32 begin_line = line;
    u32 begin_column = column;
    std::string str = ".";
    if (ptr + 2 < res.size() && res[ptr + 1] == '.' && res[ptr + 2] == '.') {
        str += "..";
    }
    ptr += str.length();
    column += str.length();
    return token {
        {begin_line, begin_column, line, column, filename},
        get_type(str),
        str
    };
}

token lexer::colons() {
    u32 begin_line = line;
    u32 begin_column = column;
    std::string str = ":";
    if (ptr + 1 < res.size() && res[ptr + 1] == ':') {
        str += ":";
    }
    ptr += str.length();
    column += str.length();
    return token {
        {begin_line, begin_column, line, column, filename},
        get_type(str),
        str
    };
}

token lexer::calc_opr() {
    u32 begin_line = line;
    u32 begin_column = column;
    // get calculation operator
    std::string str(1, res[ptr++]);
    if (ptr < res.size() && str[0] == res[ptr] && (str[0] == '&' || str[0] == '|')) {
        str += res[ptr++]; // generate && ||
    } else if (ptr < res.size() && res[ptr] == '=') {
        str += res[ptr++]; // generate _=
    }
    column += str.length();
    return token {
        {begin_line, begin_column, line, column, filename},
        get_type(str),
        str
    };
}

const error& lexer::scan(const std::string& file) {
    line = 1;
    column = 0;
    ptr = 0;
    toks = {};
    open(file);

    while (ptr < res.size()) {
        while (ptr < res.size() && skip(res[ptr])) {
            // these characters will be ignored, and '\n' will cause ++line
            ++column;
            if (res[ptr++] == '\n') {
                ++line;
                column = 0;
            }
        }
        if (ptr >= res.size()) {
            break;
        }
        if (is_note()) {
            skip_note();
        } else if (is_multi_line_comment()) {
            skip_multi_line_comment();
        } else if (is_id(res[ptr])) {
            toks.push_back(id_gen());
        } else if (is_dec(res[ptr])) {
            toks.push_back(num_gen());
        } else if (is_str(res[ptr])) {
            toks.push_back(str_gen());
        } else if (is_arrow(res[ptr]) || is_wide_arrow(res[ptr])) {
            toks.push_back(arrow_gen());
        } else if (is_single_opr(res[ptr])) {
            toks.push_back(single_opr());
        } else if (res[ptr] == '.') {
            toks.push_back(dots());
        } else if (res[ptr] == ':') {
            toks.push_back(colons());
        } else if (is_calc_opr(res[ptr])) {
            toks.push_back(calc_opr());
        } else {
            err_char();
        }
        if (invalid_char>10) {
            err.err("too many invalid characters, stop");
            break;
        }
    }
    if (toks.size()) {
        // eof token's location is the last token's location
        toks.push_back(token {toks.back().loc, tok::tk_eof, "<eof>"});
    } else {
        // if token sequence is empty, generate a default location
        toks.push_back(token {
            {line, column, line, column, filename},
            tok::tk_eof,
            "<eof>"
        });
    }
    res = "";
    return err;
}

}
