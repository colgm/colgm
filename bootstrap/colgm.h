#pragma once

#ifndef __colgm_ver__
#define __colgm_ver__ "0.2.0"
#endif

#include <cstdint>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <cmath>

#include <unistd.h>

// abbreviation of some useful basic type
using i32 = std::int32_t;
using i64 = std::int64_t;
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usize = std::size_t;
using f64 = double;

namespace colgm {

class target_info {
private:
    std::string arch = "";
    std::string platform = "";

public:
    static auto* singleton() {
        static target_info t;
        return &t;
    }

    void set_arch(const std::string& a) { arch = a; }
    void set_platform(const std::string& p) { platform = p; }
    const std::string& get_arch() const { return arch; }
    const std::string& get_platform() const { return platform; }
};

bool is_windows();
bool is_linux();
bool is_macos();
bool is_x86();
bool is_amd64();
bool is_x86_64();
bool is_arm();
bool is_aarch64();
bool is_ia64();
bool is_powerpc();
bool is_superh();
const char* get_platform();
const char* get_arch();
std::string get_cwd();

f64 hex_to_f64(const char*);
u64 hex_to_u64(const char*);
f64 oct_to_f64(const char*);
u64 oct_to_u64(const char*);
// we have the same reason not using atof here
// just as andy's interpreter does.
// it is not platform independent, and may have strange output.
// so we write a new function here to convert str to number manually.
// but this also makes 0.1+0.2==0.3,
// not another result that you may get in other languages.
f64 dec_to_f64(const char*);
u64 dec_to_u64(const char*);

f64 str_to_num(const char*);
usize utf8_hdchk(const char);
std::string char_to_hex(const char);

std::string mangle(const std::string&);

bool llvm_visible_char(char c);
std::string llvm_raw_string(const std::string&);

std::string local_time_str();

usize levenshtein_distance(const std::string&, const std::string&);

}