import subprocess
import os

def execute(cmd: list[str], allow_failure: bool, do_print: bool = True) -> int:
    if do_print:
        print("\033[92;1m[EXECUTING ]\033[0m " + " ".join(cmd), flush=True)
    ret = subprocess.run(cmd).returncode
    if ret != 0:
        if allow_failure:
            print("\033[91;1m[    Error ]\033[0m [ignored]" + " ".join(cmd), flush=True)
        if not allow_failure:
            print("\033[91;1m[    Error ]\033[0m [exited]" + " ".join(cmd), flush=True)
            exit(ret)
    return ret

TEST_LIST = [
    # file name                | library path   | allowed to fail
    ("test/align.colgm",                   "src", False),
    ("test/array_type.colgm",              "src", False),
    ("test/array.colgm",                   "src", False),
    ("test/assign.colgm",                  "src", False),
    ("test/bitwise.colgm",                 "src", False),
    ("test/branch.colgm",                  "src", False),
    ("test/cmpnot.colgm",                  "src", False),
    ("test/complex_generics.colgm",        "src", False),
    ("test/continue_break.colgm",          "src", False),
    ("test/defer.colgm",                   "src", False),
    ("test/enum_test.colgm",               "src", False),
    ("test/for_iter.colgm",                "src", False),
    ("test/for_test.colgm",                "src", False),
    ("test/func.colgm",                    "src", False),
    ("test/generic_embed.colgm",           "src", False),
    ("test/hello.colgm",                   "src", False),
    ("test/initializer.colgm",             "src", False),
    ("test/local.colgm",                   "src", False),
    ("test/match.colgm",                   "src", False),
    ("test/negative.colgm",                "src", False),
    ("test/recursive_instantiation.colgm", "src", True),
    ("test/report_const_changed.colgm",    "src", True),
    ("test/self_ref_test.colgm",           "src", True),
    ("test/std_test.colgm",                "src", False),
    ("test/string.colgm",                  "src", False),
    ("test/tagged_union.colgm",            "src", False),
    ("test/to_str.colgm",                  "src", False),
    ("test/type_convert.colgm",            "src", False),
    ("test/unreachable.colgm",             "src", False),
    ("test/void_return.colgm",             "src", False),
    ("test/warn_on_left_call.colgm",       "src", False)
]

for (test, lib, allow_failure) in TEST_LIST:
    ret = execute([
        "./build/colgm_self_host",
        test,
        "-L", lib,
        "-o", "test.out",
        "-O2",
        "-g"
    ], allow_failure)
    if ret != 0:
        continue
    execute(["./test.out"], allow_failure, False)

if os.path.exists("test.out"):
    execute(["rm", "test.out"], False, False)
if os.path.exists("test.out.ll"):
    execute(["rm", "test.out.ll"], False, False)
