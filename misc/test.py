import subprocess
import os

def execute(cmd: list[str], allow_failure: bool, do_print: bool = True) -> int:
    if do_print:
        print("Executing: " + " ".join(cmd), flush=True)
    ret = subprocess.run(cmd).returncode
    if ret != 0:
        print("Error: " + " ".join(cmd), flush=True)
        if not allow_failure:
            exit(ret)
    return ret

TEST_LIST = [
    # file name                | library path   | allowed to fail
    ("test/assign.colgm",                  "src", False),
    ("test/bitwise.colgm",                 "src", False),
    ("test/branch.colgm",                  "src", False),
    ("test/cmpnot.colgm",                  "src", False),
    ("test/complex_generics.colgm",        "src", False),
    ("test/continue_break.colgm",          "src", False),
    ("test/enum_test.colgm",               "src", False),
    ("test/for_iter.colgm",                "src", False),
    ("test/for_test.colgm",                "src", False),
    ("test/func.colgm",                    "src", False),
    ("test/generic_array.colgm",           "src", False),
    ("test/generic_embed.colgm",           "src", False),
    ("test/hello.colgm",                   "src", False),
    ("test/initializer.colgm",             "src", False),
    ("test/local.colgm",                   "src", False),
    ("test/match.colgm",                   "src", False),
    ("test/negative.colgm",                "src", False),
    ("test/recursive_instantiation.colgm", "src", True),
    ("test/self_ref_test.colgm",           "src", False),
    ("test/std_test.colgm",                "src", False),
    ("test/string.colgm",                  "src", False),
    ("test/to_str.colgm",                  "src", False),
    ("test/type_convert.colgm",            "src", False),
    ("test/void_return.colgm",             "src", False),
    ("test/warn_on_left_call.colgm",       "src", False)
]

for (test, lib, allow_failure) in TEST_LIST:
    ret = execute([
        "./build/colgm_self_host",
        test,
        "--library", lib,
        "-o", "test.out",
        "-g", "-O2"
    ], allow_failure)
    if ret != 0:
        continue
    execute(["./test.out"], allow_failure)

if os.path.exists("test.out"):
    execute(["rm", "test.out"], False, False)
if os.path.exists("test.out.ll"):
    execute(["rm", "test.out.ll"], False, False)
