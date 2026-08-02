import subprocess
import os
import difflib

def red(s: str) -> str:
    if not os.isatty(1):
        return s
    return "\033[91m" + s + "\033[0m"

def green(s: str) -> str:
    if not os.isatty(1):
        return s
    return "\033[92m" + s + "\033[0m"

def yellow(s: str) -> str:
    if not os.isatty(1):
        return s
    return "\033[93m" + s + "\033[0m"

def show_diff(expect: str, actual: str):
    diff = difflib.ndiff(
        expect.splitlines(),
        actual.splitlines()
    )

    print(yellow("==================== actual vs expected ===================="))
    for line in diff:
        if line.startswith("+"):
            print(green(line))
        elif line.startswith("-"):
            print(red(line))
        elif line.startswith("?"):
            print(yellow(line))
        else:
            print(line)
    print(yellow("========================= end diff ========================="))

def execute(cmd: list[str]) -> str:
    print("\033[92;1m EXECUTING \033[0m" + " ".join(cmd), flush=True)
    ret = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    err = ret.stderr.decode()
    return err

TEST_LIST = []

for root, dirs, files in os.walk("test/error"):
    for f in files:
        if f.endswith(".colgm"):
            TEST_LIST.append(os.path.join(root, f[:-6]))

TEST_LIST.sort()
for test in TEST_LIST:
    source_code = test + ".colgm"
    result_text = test + ".result"
    err_report = execute([
        "build/colgm_self_host",
        "-L", "src",
        source_code,
        "-emit-llvm",
        "-o", "test_error.out"
    ])
    err_report = err_report.replace("\r", "")
    if not os.path.exists(result_text):
        continue
    with open(result_text, "r") as fp:
        result = fp.read().replace("\r", "")
        if len(result) == 0:
            print("\033[91;1m    FAILED\033[0m empty test result")
            print("\033[91;1m    FAILED\033[0m got:")
            show_diff("", err_report)
            continue
        if result != err_report:
            print("\033[91;1m    FAILED\033[0m expected:")
            show_diff(result, err_report)
            exit(1)

if os.path.exists("test_error.out.ll"):
    os.remove("test_error.out.ll")
if os.path.exists("test_error.out"):
    os.remove("test_error.out")