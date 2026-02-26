#include "colgm.h"
#include "lexer.h"
#include "parse/parse.h"
#include "ast/dumper.h"
#include "sema/semantic.h"
#include "mir/ast2mir.h"
#include "mir/pass_manager.h"
#include "sir/mir2sir.h"
#include "sir/pass_manager.h"
#include "package/package.h"

#include <vector>
#include <unordered_map>
#include <thread>
#include <cstdlib>

const u32 COMPILE_VIEW_TOKEN = 1;
const u32 COMPILE_VIEW_AST = 1<<1;
const u32 COMPILE_VIEW_LIB = 1<<2;
const u32 COMPILE_VIEW_SEMA = 1<<3;
const u32 COMPILE_VIEW_SIR = 1<<4;
const u32 COMPILE_VIEW_MIR = 1<<5;
const u32 COMPILE_VIEW_PASS = 1<<6;

std::ostream& help(std::ostream& out) {
    out
    << "\ncolgm <option>\n"
    << "option:\n"
    << "   -h,   --help           | get help.\n"
    << "   -v,   --version        | get version.\n"
    << "\ncolgm [option] <file>\n"
    << "option:\n"
    << "   -o,   --output <file>  | output file, default <a.out.ll>.\n"
    << "   -l,   --lex            | view analysed tokens.\n"
    << "   -a,   --ast            | view ast.\n"
    << "   -s,   --sema           | view semantic result.\n"
    << "         --mir            | view mir.\n"
    << "         --sir            | view sir.\n"
    << "   -L,   --library <path> | add library path.\n"
    << "         --dump-lib       | view libraries.\n"
    << "         --pass-info      | view pass info.\n"
    << "         --arch           | specify target arch.\n"
    << "         --platform       | specify target platform.\n"
    << "file:\n"
    << "   <filename>             | input file.\n"
    << "\n";
    return out;
}

std::ostream& logo(std::ostream& out) {
    out
    << "                     __             \n"
    << "         _________  / /___  ___ __  \n"
    << "        / ___/ __ \\/ / __ `/ _ `_ \\ \n"
    << "       / /__/ /_/ / / /_/ / // // / \n"
    << "       \\___/\\____/_/\\__, /_//_//_/  \n"
    << "      -------------/____/-----------\n\n"
    << "version : " << __colgm_ver__
    << " " << colgm::get_platform() << " " << colgm::get_arch()
    << " (" << __DATE__ << " " << __TIME__ << ")\n"
    << "repo    : https://github.com/colgm/colgm\n"
    << "license : Apache 2.0\n"
    << "\n"
    << "input <colgm -h> to get help.\n\n";
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

void scan_package(const std::string& library_path,
                  const std::string& entry_file,
                  const u32 cmd) {
    if (!library_path.empty()) {
        colgm::package_manager::singleton()->set_library_path(library_path);
    } else {
        colgm::package_manager::singleton()->set_library_path(".");
    }
    colgm::package_manager::singleton()->generate_search_order();
    if (cmd & COMPILE_VIEW_LIB) {
        colgm::package_manager::singleton()->dump_search_order();
    }
}

void execute(const std::string& input_file,
             const std::string& output_file,
             const u32 cmd = 0) {
    // main components of compiler
    colgm::error err;
    colgm::lexer lexer(err);
    colgm::parse parser(err);
    colgm::semantic sema(err);
    sema.set_main_input_file(input_file);
    colgm::mir::ast2mir ast2mir(err, sema.get_context());

    // ast -> mir -> sir generator pass
    colgm::mir::pass_manager mpm;
    colgm::mir::mir2sir mir2sir(sema.get_context());
    colgm::sir_pass_manager spm;

    // lexer scans file to get tokens
    lexer.scan(input_file).chkerr();
    if (cmd & COMPILE_VIEW_TOKEN) {
        for (const auto& token : lexer.result()) {
            std::cout << token.loc << ": " << token.str << "\n";
        }
    }

    // parser
    parser.analyse(lexer.result()).chkerr();
    if (cmd & COMPILE_VIEW_AST) {
        colgm::ast::dumper::dump(parser.get_result());
    }

    // simple semantic
    const auto& r = sema.analyse(parser.get_result(), cmd & COMPILE_VIEW_PASS);
    // still dump semantic symbol whatever happened
    if (cmd & COMPILE_VIEW_SEMA) {
        sema.dump();
    }
    r.chkerr();

    // generate mir code
    ast2mir.generate(parser.get_result()).chkerr();
    mpm.execute(colgm::mir::ast2mir::get_context(), cmd & COMPILE_VIEW_PASS);
    if (cmd & COMPILE_VIEW_MIR) {
        colgm::mir::ast2mir::dump(std::cout);
    }

    // generate sir code
    mir2sir.generate(*ast2mir.get_context()).chkerr();
    if (!spm.execute(&mir2sir.get_mutable_sir_context(), cmd & COMPILE_VIEW_PASS)) {
        std::exit(1);
    }
    if (cmd & COMPILE_VIEW_SIR) {
        mir2sir.get_mutable_sir_context().dump_code(std::cout);
    }

    std::ofstream out(output_file);
    mir2sir.get_mutable_sir_context().dump_code(out);
}

i32 main(i32 argc, const char* argv[]) {
    // output version info
    if (argc <= 1) {
        std::clog << logo;
        return 0;
    }

    // run directly or show help
    if (argc == 2) {
        std::string s(argv[1]);
        if (s == "-h" || s == "--help") {
            std::clog << help;
        } else if (s == "-v" || s == "--version") {
            std::clog << version;
        } else if (s[0] != '-') {
            scan_package(".", s, 0);
            execute(s, "a.out.ll");
        } else {
            err();
        }
        return 0;
    }

    // execute with arguments
    const std::unordered_map<std::string, u32> cmdlst = {
        {"--lex", COMPILE_VIEW_TOKEN},
        {"-l", COMPILE_VIEW_TOKEN},
        {"--ast", COMPILE_VIEW_AST},
        {"-a", COMPILE_VIEW_AST},
        {"--sema", COMPILE_VIEW_SEMA},
        {"-s", COMPILE_VIEW_SEMA},
        {"--mir", COMPILE_VIEW_MIR},
        {"--sir", COMPILE_VIEW_SIR},
        {"--dump-lib", COMPILE_VIEW_LIB},
        {"--pass-info", COMPILE_VIEW_PASS},
    };
    u32 cmd = 0;
    std::string input_file = "";
    std::string output_file = "a.out.ll";
    std::string library_path = "";

    std::vector<std::string> args;
    for (i32 i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    for (i32 i = 1; i < argc; ++i) {
        if (args[i] == "-h" || args[i] == "--help") {
            std::clog << help;
            return 0;
        } else if (cmdlst.count(args[i])) {
            cmd |= cmdlst.at(args[i]);
        } else if (args[i] == "-L" || args[i] == "--library") {
            if (i + 1 < argc) {
                library_path = args[i + 1];
                ++i;
            } else {
                err();
            }
        } else if (args[i] == "-o" || args[i] == "--output") {
            if (i + 1 < argc) {
                output_file = args[i + 1];
                ++i;
            } else {
                err();
            }
        } else if (args[i] == "--arch") {
            if (i + 1 < argc) {
                colgm::target_info::singleton()->set_arch(args[i + 1]);
                ++i;
            } else {
                err();
            }
        } else if (args[i] == "--platform") {
            if (i + 1 < argc) {
                colgm::target_info::singleton()->set_platform(args[i + 1]);
                ++i;
            } else {
                err();
            }
        } else if (!input_file.length()) {
            input_file = args[i];
        } else {
            err();
        }
    }

    // input file must be specified
    if (!input_file.length()) {
        err();
    }

    scan_package(library_path, input_file, cmd);
    execute(input_file, output_file, cmd);
    return 0;
}