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
        print("\033[91;1m     ERROR \033[0m" + " ".join(cmd), flush=True)
    return ret

TEST_LIST = [
    # file name                            | argv
    ("test/align.colgm",                   []),
    ("test/array_type.colgm",              []),
    ("test/array.colgm",                   []),
    ("test/assign.colgm",                  []),
    ("test/base64_test.colgm",             []),
    ("test/bitwise.colgm",                 []),
    ("test/branch.colgm",                  []),
    ("test/cmpnot.colgm",                  []),
    ("test/complex_generics.colgm",        []),
    ("test/const_fold.colgm",              []),
    ("test/continue_break.colgm",          []),
    ("test/defer.colgm",                   []),
    ("test/enum_test.colgm",               []),
    ("test/errno.colgm",                   []),
    ("test/for_iter.colgm",                []),
    ("test/for_test.colgm",                []),
    ("test/func.colgm",                    []),
    ("test/generic_embed.colgm",           []),
    ("test/hello.colgm",                   []),
    ("test/initializer.colgm",             []),
    ("test/json_test.colgm",               []),
    ("test/list_dir.colgm",                ["src"]),
    ("test/local.colgm",                   []),
    ("test/match.colgm",                   []),
    ("test/md5_test.colgm",                []),
    ("test/negative.colgm",                []),
    ("test/ref_struct_field.colgm",        []),
    ("test/ref_variable_assign.colgm",     []),
    ("test/std_test.colgm",                []),
    ("test/string.colgm",                  []),
    ("test/tagged_union.colgm",            []),
    ("test/to_str.colgm",                  []),
    ("test/type_convert.colgm",            []),
    ("test/utf8_test.colgm",               []),
    ("test/void_return.colgm",             []),
    ("test/warn_on_left_call.colgm",       [])
]

COMPILER = "build/colgm_self_host"
if sys.platform == "win32":
    COMPILER = "cmake-windows-build\\colgm_self_host.exe"

for (test, argv) in TEST_LIST:
    ret = execute([
        COMPILER,
        test,
        "-o", "test.out",
        "-O2",
        "-g"
    ])
    if ret != 0:
        break
    ret = execute(["./test.out"] + argv, False)
    if ret != 0:
        break
    time.sleep(0.5)

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
