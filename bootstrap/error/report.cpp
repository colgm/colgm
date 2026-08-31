#include "error/report.h"
#include "utils/colorful.h"

namespace colgm {

void flstream::load(const std::string& f) {
    if (file == f) { // don't need to load a loaded file
        return;
    }

    // update file name
    file = f;

    res.clear();
    std::ifstream in(f, std::ios::binary);
    if (in.fail()) {
        std::cerr << red << "src: " << reset << "cannot open <" << f << ">\n";
        std::exit(1);
    }

    while (!in.eof()) {
        std::string line;
        std::getline(in, line);
        res.push_back(line);
    }
}

void error::err(const std::string& info) {
    ++cnt;
    std::cerr << red << "Error: " << reset << info << "\n\n";
}

void error::warn(const std::string& info) {
    std::clog << orange << "Warning: " << reset << info << "\n\n";
}

void error::err(const span& loc, const std::string& info, const std::string& note) {
    // load error occurred file into string lines
    load(loc.file);

    ++cnt;

    std::cerr
    << red << "Error: " << reset << info << "\n" << cyan << "  --> "
    << red << loc << reset << "\n";

    const usize maxlen = std::to_string(loc.end_line).length();
    const std::string iden = identation(maxlen);

    std::cerr << cyan << iden << " | " << reset << "\n";

    for (u32 line = loc.begin_line; line <= loc.end_line; ++line) {
        // skip line 0
        if (!line) {
            continue;
        }

        if (loc.begin_line < line && line < loc.end_line) {
            if (line == loc.begin_line + 1) {
                std::cerr << cyan << iden << " | " << reset << "...\n";
                std::cerr << cyan << iden << " | " << reset << "\n";
            }
            continue;
        }

        // line out of range
        if (line - 1 >= res.size()) {
            continue;
        }

        // if this line has nothing, skip
        if (!res[line - 1].length() && line != loc.end_line) {
            continue;
        }

        const auto& code = res[line - 1];
        std::cerr << cyan << leftpad(line, maxlen) << " | " << reset << code << "\n";
        // output underline
        std::cerr << cyan << iden << " | " << reset;
        if (loc.begin_line == loc.end_line) {
            for (u32 i = 0; i < loc.begin_column; ++i) {
                std::cerr << char(" \t"[code[i] == '\t']);
            }
            for (u32 i = loc.begin_column; i < loc.end_column; ++i) {
                std::cerr << red << (code[i] == '\t' ? "^^^^" : "^") << reset;
            }
        } else if (line == loc.begin_line) {
            for (u32 i = 0; i < loc.begin_column; ++i) {
                std::cerr << char(" \t"[code[i] == '\t']);
            }
            for (u32 i = loc.begin_column; i < code.size(); ++i) {
                std::cerr << red << (code[i] == '\t' ? "^^^^" : "^") << reset;
            }
        } else if (loc.begin_line < line && line < loc.end_line) {
            for (u32 i = 0; i < code.size(); ++i) {
                std::cerr << red << (code[i] == '\t' ? "^^^^" : "^");
            }
        } else {
            for (u32 i = 0; i < loc.end_column; ++i) {
                std::cerr << red << (code[i] == '\t' ? "^^^^" : "^");
            }
        }
        if (line == loc.end_line) {
            std::cerr << reset;
        } else {
            std::cerr << reset << "\n";
        }
    }
    if (note.size()) {
        std::cerr << "\n" << iden << cyan << "note: " << reset << note;
    }
    std::cerr << "\n\n";
}

void error::warn(const span& loc, const std::string& info, const std::string& note) {
    // load error occurred file into string lines
    load(loc.file);

    std::cerr
    << orange << "Warning: " << reset << info << "\n" << cyan << "  --> "
    << orange << loc << reset << "\n";

    const usize maxlen = std::to_string(loc.end_line).length();
    const std::string iden = identation(maxlen);

    for (u32 line = loc.begin_line; line <= loc.end_line; ++line) {
        // skip line 0
        if (!line) {
            continue;
        }

        if (loc.begin_line < line && line < loc.end_line) {
            if (line == loc.begin_line + 1) {
                std::cerr << cyan << iden << " | " << reset << "...\n";
                std::cerr << cyan << iden << " | " << reset << "\n";
            }
            continue;
        }

        // line out of range
        if (line - 1 >= res.size()) {
            continue;
        }

        // if this line has nothing, skip
        if (!res[line - 1].length() && line != loc.end_line) {
            continue;
        }

        const auto& code = res[line - 1];
        std::cerr << cyan << leftpad(line, maxlen) << " | " << reset << code << "\n";
        // output underline
        std::cerr << cyan << iden << " | " << reset;
        if (loc.begin_line == loc.end_line) {
            for (u32 i = 0; i < loc.begin_column; ++i) {
                std::cerr << char(" \t"[code[i] == '\t']);
            }
            for (u32 i = loc.begin_column; i < loc.end_column; ++i) {
                std::cerr << orange << (code[i] == '\t' ? "^^^^" : "^") << reset;
            }
        } else if (line == loc.begin_line) {
            for (u32 i = 0; i < loc.begin_column; ++i) {
                std::cerr << char(" \t"[code[i] == '\t']);
            }
            for (u32 i = loc.begin_column; i < code.size(); ++i) {
                std::cerr << orange << (code[i] == '\t' ? "^^^^" : "^") << reset;
            }
        } else if (loc.begin_line < line && line < loc.end_line) {
            for (u32 i = 0; i<code.size(); ++i) {
                std::cerr << orange << (code[i] == '\t' ? "^^^^" : "^");
            }
        } else {
            for (u32 i = 0; i < loc.end_column; ++i) {
                std::cerr << orange << (code[i] == '\t' ? "^^^^" : "^");
            }
        }
        if (line == loc.end_line) {
            std::cerr << reset;
        } else {
            std::cerr << reset << "\n";
        }
    }
    if (note.size()) {
        std::cerr << "\n" << iden << cyan << "note: " << reset << note;
    }
    std::cerr << "\n\n";
}

}
