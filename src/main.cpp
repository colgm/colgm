#include "colgm.h"
#include "err.h"
#include "lexer.h"

#include <unordered_map>
#include <thread>
#include <cstdlib>

const u32 COMPILE_VIEW_TOKEN = 1;

std::ostream& help(std::ostream& out) {
    out
    << "\n"
    << "     ,--#-,\n"
    << "<3  / \\____\\  <3\n"
    << "    |_|__A_|\n"
#ifdef _WIN32
    << "use command <chcp 65001> to use unicode.\n"
#endif
    << "\ncolgm <option>\n"
    << "option:\n"
    << "   -h,   --help     | get help.\n"
    << "   -v,   --version  | get version.\n"
    << "\ncolgm [option] <file> [argv]\n"
    << "option:\n"
    << "   -l,   --lex      | view analysed tokens.\n"
    << "file:\n"
    << "   <filename>       | imput file.\n"
    << "argv:\n"
    << "   <args>           | cmd arguments used in program.\n"
    << "\n";
    return out;
}

std::ostream& logo(std::ostream& out) {
    out
    << "colgm compiler\n"
    << "ver : " << __colgm_ver__
    << " " << colgm::get_platform() << " " << colgm::get_arch()
    << " (" << __DATE__ << " " << __TIME__ << ")\n"
    << "std : c++ " << __cplusplus << "\n"
    << "core: " << std::thread::hardware_concurrency() << " core(s)\n"
    << "\n"
    << "input <colgm -h> to get help .\n\n";
    return out;
}

std::ostream& version(std::ostream& out) {
    out << "colgm compiler version " << __colgm_ver__;
    out << " " << colgm::get_platform() << " " << colgm::get_arch();
    out << " (" << __DATE__ << " " << __TIME__ << ")\n";
    return out;
}

[[noreturn]]
void err() {
    std::cerr
    << "invalid argument(s).\n"
    << "use <colgm -h> to get help.\n";
    std::exit(1);
}

void execute(
    const std::string& file,
    const std::vector<std::string>& argv,
    const u32 cmd = 0) {

    using clk = std::chrono::high_resolution_clock;
    const auto den = clk::duration::period::den;

    colgm::lexer lex;

    // lexer scans file to get tokens
    lex.scan(file).chkerr();
    if (cmd&COMPILE_VIEW_TOKEN) {
        for(const auto& token : lex.result()) {
            std::cout << token.loc << ": " << token.str << "\n";
        }
    }
}

i32 main(i32 argc, const char* argv[]) {
    // output version info
    if (argc<=1) {
        std::clog << logo;
        return 0;
    }

    // run directly or show help
    if (argc==2) {
        std::string s(argv[1]);
        if (s=="-h" || s=="--help") {
            std::clog << help;
        } else if (s=="-v" || s=="--version") {
            std::clog << version;
        }else if (s[0]!='-') {
            execute(s, {});
        } else {
            err();
        }
        return 0;
    }

    // execute with arguments
    const std::unordered_map<std::string, u32> cmdlst = {
        {"--lex", COMPILE_VIEW_TOKEN},
        {"-l", COMPILE_VIEW_TOKEN},
    };
    u32 cmd = 0;
    std::string filename = "";
    std::vector<std::string> vm_argv;
    for(i32 i = 1; i<argc; ++i) {
        if (cmdlst.count(argv[i])) {
            cmd |= cmdlst.at(argv[i]);
        } else if (!filename.length()) {
            filename = argv[i];
        } else {
            vm_argv.push_back(argv[i]);
        }
    }
    if (!filename.length()) {
        err();
    }
    execute(filename, vm_argv, cmd);
    return 0;
}