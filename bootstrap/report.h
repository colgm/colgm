#pragma once

#include <iostream>
#include <fstream>
#include <sstream> // MSVC need this to use std::getline
#include <cstring>
#include <vector>

#include "colgm.h"

namespace colgm {

struct span {
    u32 begin_line;
    u32 begin_column;
    u32 end_line;
    u32 end_column;
    std::string file;

    static const span null() {
        return {0, 0, 0, 0, ""};
    }
};

std::ostream& operator<<(std::ostream&, const span&);

std::ostream& back_white(std::ostream&);
std::ostream& red(std::ostream&);
std::ostream& cyan(std::ostream&);
std::ostream& green(std::ostream&);
std::ostream& orange(std::ostream&);
std::ostream& white(std::ostream&);
std::ostream& reset(std::ostream&);

class flstream {
protected:
    std::string file;
    std::vector<std::string> res;

public:
    flstream():file("") {}
    void load(const std::string&);
    const auto& operator[](usize n) const { return res[n]; }
    const auto& name() const { return file; }
    const auto& file_content() const { return res; }
    auto size() const { return res.size(); }
};

class error: public flstream {
private:
    u64 cnt; // counter for errors

    std::string identation(usize len) {
        return std::string(len, ' ');
    }
    std::string leftpad(u32 num, usize len) {
        auto tmp = std::to_string(num);
        while (tmp.length()<len) {
            tmp = " " + tmp;
        }
        return tmp;
    }

public:
    error(): cnt(0) {}
    void err(const std::string&);
    void warn(const std::string&);
    void err(const span&, const std::string&);
    void warn(const span&, const std::string&);

    void chkerr() const {
        if (cnt) {
            std::exit(1);
        }
    }
    auto geterr() const { return cnt; }
};

}
