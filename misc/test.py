import subprocess
import os

def execute(cmd: list[str], allow_failure: bool) -> int:
    print("Executing: " + " ".join(cmd), flush=True)
    ret = subprocess.run(cmd).returncode
    if ret != 0:
        print("Error: " + " ".join(cmd), flush=True)
        if not allow_failure:
            exit(ret)
    return ret

TEST_LIST = [
    # file name                      | library path   | allowed to fail
    ("test/assign.colgm",                  "test/test_lib", False),
    ("test/bitwise.colgm",                 "test/test_lib", False),
    ("test/branch.colgm",                  "test/test_lib", False),
    ("test/cmpnot.colgm",                  "test/test_lib", False),
    ("test/complex_generics.colgm",        "test/test_lib", False),
    ("test/continue_break.colgm",          "test/test_lib", False),
    ("test/enum_test.colgm",               "test/test_lib", False),
    ("test/for_iter.colgm",                "src",           False),
    ("test/for_test.colgm",                "test/test_lib", False),
    ("test/func.colgm",                    "test/test_lib", False),
    ("test/generic_array.colgm",           "test/test_lib", False),
    ("test/generic_embed.colgm",           "test/test_lib", False),
    ("test/generic.colgm",                 "test/test_lib", False),
    ("test/hello.colgm",                   "test/test_lib", False),
    ("test/initializer.colgm",             "test/test_lib", False),
    ("test/local.colgm",                   "test/test_lib", False),
    ("test/match.colgm",                   "test/test_lib", False),
    ("test/negative.colgm",                "test/test_lib", False),
    ("test/recursive_instantiation.colgm", "test/test_lib", True),
    ("test/self_ref_test.colgm",           "test/test_lib", False),
    ("test/string.colgm",                  "test/test_lib", False),
    ("test/to_str.colgm",                  "test/test_lib", False),
    ("test/type_convert.colgm",            "test/test_lib", False),
    ("test/void_return.colgm",             "test/test_lib", False),
    ("test/warn_on_left_call.colgm",       "test/test_lib", False),
    ("src/test/test.colgm",                "src",           False)
]

for (test, lib, allow_failure) in TEST_LIST:
    ret = execute([
        "./a.out",
        test,
        "--library", lib,
        "-o", "test.out",
        "-g", "-O2"
    ], allow_failure)
    if ret != 0:
        continue
    execute(["./test.out"], allow_failure)

if os.path.exists("test.out"):
    execute(["rm", "test.out"], False)
if os.path.exists("test.out.ll"):
    execute(["rm", "test.out.ll"], False)