#include "colgm.h"
#include "lexer.h"
#include "parse.h"
#include "ast/dumper.h"
#include "sema/semantic.h"
#include "mir/ast2mir.h"
#include "mir/pass_manager.h"
#include "mir/mir2sir.h"
#include "sir/pass_manager.h"
#include "package/package.h"

#include <unordered_map>
#include <thread>
#include <cstdlib>

const u32 COMPILE_VIEW_TOKEN = 1;
const u32 COMPILE_VIEW_AST = 1<<1;
const u32 COMPILE_VIEW_LIB = 1<<2;
const u32 COMPILE_VIEW_SEMA = 1<<3;
const u32 COMPILE_VIEW_SIR = 1<<4;
const u32 COMPILE_VIEW_MIR = 1<<5;

std::ostream& help(std::ostream& out) {
    out
    << "\ncolgm <option>\n"
    << "option:\n"
    << "   -h,   --help           | get help.\n"
    << "   -v,   --version        | get version.\n"
    << "\ncolgm [option] <file>\n"
    << "option:\n"
    << "   -l,   --lex            | view analysed tokens.\n"
    << "   -a,   --ast            | view ast.\n"
    << "   -s,   --sema           | view semantic result.\n"
    << "         --mir            | view mir.\n"
    << "         --sir            | view sir.\n"
    << "   -L,   --library <path> | add library path.\n"
    << "         --dump-lib       | view libraries.\n"
    << "file:\n"
    << "   <filename>             | input file.\n"
    << "\n";
    return out;
}

std::ostream& logo(std::ostream& out) {
    out
    << "                __              \n"
    << "    _________  / /___ _____ ___ \n"
    << "   / ___/ __ \\/ / __ `/ __ `__ \\\n"
    << "  / /__/ /_/ / / /_/ / / / / / /\n"
    << "  \\___/\\____/_/\\__, /_/ /_/ /_/ \n"
    << "              /____/            \n\n"
    << "ver : " << __colgm_ver__
    << " " << colgm::get_platform() << " " << colgm::get_arch()
    << " (" << __DATE__ << " " << __TIME__ << ")\n"
    << "std : c++ " << __cplusplus << "\n"
    << "core: " << std::thread::hardware_concurrency() << " core(s)\n"
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
    if (library_path.empty()) {
        return;
    }
    colgm::package_manager::singleton()->scan(library_path, entry_file).chkerr();
    if (cmd&COMPILE_VIEW_LIB) {
        colgm::package_manager::singleton()->dump_packages();
    }
}

void execute(const std::string& file,
             const u32 cmd = 0) {
    // main components of compiler
    colgm::error err;
    colgm::lexer lexer(err);
    colgm::parse parser(err);
    colgm::semantic sema(err);
    colgm::mir::ast2mir ast2mir(err, sema.get_context());

    // ast -> mir -> sir generator pass
    colgm::mir::pass_manager mpm;
    colgm::mir::mir2sir mir2sir(sema.get_context());
    colgm::sir_pass_manager spm;

    // lexer scans file to get tokens
    lexer.scan(file).chkerr();
    if (cmd&COMPILE_VIEW_TOKEN) {
        for(const auto& token : lexer.result()) {
            std::cout << token.loc << ": " << token.str << "\n";
        }
    }

    // parser
    parser.analyse(lexer.result()).chkerr();
    if (cmd&COMPILE_VIEW_AST) {
        colgm::dumper dump;
        parser.get_result()->accept(&dump);
    }

    // simple semantic
    sema.analyse(parser.get_result()).chkerr();
    if (cmd&COMPILE_VIEW_SEMA) {
        sema.dump();
    }

    // generate mir code
    ast2mir.generate(parser.get_result()).chkerr();
    mpm.execute(colgm::mir::ast2mir::get_context());
    if (cmd&COMPILE_VIEW_MIR) {
        colgm::mir::ast2mir::dump(std::cout);
    }

    // generate sir code
    mir2sir.generate(*ast2mir.get_context()).chkerr();
    spm.execute(&mir2sir.get_mutable_sir_context());
    if (cmd&COMPILE_VIEW_SIR) {
        mir2sir.get_mutable_sir_context().dump_code(std::cout);
    }

    std::ofstream out("out.ll");
    mir2sir.get_mutable_sir_context().dump_code(out);
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
        } else if (s[0]!='-') {
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
        {"--ast", COMPILE_VIEW_AST},
        {"-a", COMPILE_VIEW_AST},
        {"--sema", COMPILE_VIEW_SEMA},
        {"-s", COMPILE_VIEW_SEMA},
        {"--mir", COMPILE_VIEW_MIR},
        {"--sir", COMPILE_VIEW_SIR},
        {"--dump-lib", COMPILE_VIEW_LIB}
    };
    u32 cmd = 0;
    std::string filename = "";
    std::string library_path = "";
    for(i32 i = 1; i<argc; ++i) {
        if (argv[i]==std::string("-h") || argv[i]==std::string("--help")) {
            std::clog << help;
            break;
        } else if (cmdlst.count(argv[i])) {
            cmd |= cmdlst.at(argv[i]);
        } else if (argv[i]==std::string("-L") || argv[i]==std::string("--library")) {
            if (i+1<argc) {
                library_path = argv[i+1];
                ++i;
            }
        } else if (!filename.length()) {
            filename = argv[i];
        } else {
            err();
        }
    }
    if (!filename.length()) {
        err();
    }

    if (library_path.empty()) {
        library_path = ".";
    }

    scan_package(library_path, filename, cmd);
    execute(filename, cmd);
    return 0;
}