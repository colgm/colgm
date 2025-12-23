import subprocess
import os
import sys
import shutil
import time

def execute(cmd: list[str], do_print: bool = True) -> int:
    if do_print:
        print("\033[92;1m EXECUTING \033[0m" + " ".join(cmd), flush=True)
    ret = subprocess.run(cmd).returncode
    if ret != 0:
        print("\033[91;1m     Error \033[0m[exited]" + " ".join(cmd), flush=True)
        exit(ret)
    return ret

TEST_LIST = [
    # file name                            | library path
    ("test/align.colgm",                   "src"),
    ("test/array_type.colgm",              "src"),
    ("test/array.colgm",                   "src"),
    ("test/assign.colgm",                  "src"),
    ("test/bitwise.colgm",                 "src"),
    ("test/branch.colgm",                  "src"),
    ("test/cmpnot.colgm",                  "src"),
    ("test/complex_generics.colgm",        "src"),
    ("test/continue_break.colgm",          "src"),
    ("test/defer.colgm",                   "src"),
    ("test/enum_test.colgm",               "src"),
    ("test/errno.colgm",                   "src"),
    ("test/for_iter.colgm",                "src"),
    ("test/for_test.colgm",                "src"),
    ("test/func.colgm",                    "src"),
    ("test/generic_embed.colgm",           "src"),
    ("test/hello.colgm",                   "src"),
    ("test/initializer.colgm",             "src"),
    ("test/json_test.colgm",               "src"),
    ("test/local.colgm",                   "src"),
    ("test/match.colgm",                   "src"),
    ("test/md5_test.colgm",                "src"),
    ("test/negative.colgm",                "src"),
    ("test/ref_struct_field.colgm",        "src"),
    ("test/ref_variable_assign.colgm",     "src"),
#    ("test/sleep.colgm",                   "src"),
    ("test/std_test.colgm",                "src"),
    ("test/string.colgm",                  "src"),
    ("test/tagged_union.colgm",            "src"),
    ("test/to_str.colgm",                  "src"),
    ("test/type_convert.colgm",            "src"),
    ("test/void_return.colgm",             "src"),
    ("test/warn_on_left_call.colgm",       "src")
]

COMPILER = "build/colgm_self_host"
if sys.platform == "win32":
    COMPILER = "cmake-windows-build\\colgm_self_host.exe"

for (test, lib) in TEST_LIST:
    ret = execute([
        COMPILER,
        test,
        "-L", lib,
        "-o", "test.out",
        "-O2",
        "-g"
    ])
    if ret != 0:
        continue
    execute(["./test.out"], False)
    time.sleep(0.1)

if os.path.exists("test.out"):
    os.remove("test.out")
if os.path.exists("test.out.ll"):
    os.remove("test.out.ll")
if os.path.exists("test.out.dSYM"):
    shutil.rmtree("test.out.dSYM")
if os.path.exists("test.pdb"):
    os.remove("test.pdb")
if os.path.exists("test.ilk"):
    os.remove("test.ilk")
