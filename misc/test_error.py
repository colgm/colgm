import subprocess
import os

def execute(cmd: list[str]) -> str:
    print("\033[92;1m EXECUTING \033[0m" + " ".join(cmd), flush=True)
    ret = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    err = ret.stderr.decode()
    return err

def de_color(text: str) -> str:
    color = [
        "0", "0;1", "1", "90", "90;1",
        "91", "91;1", "92", "92;1",
        "93", "93;1", "94", "94;1",
        "95", "95;1", "96", "96;1",
        "97", "97;1"
    ]
    for i in color:
        text = text.replace("\033[" + i + "m", "")
    text = text.replace("\r", "")
    text = text.replace("\t", "")
    text = text.replace(" ", "")
    return text

TEST_LIST = []

for root, dirs, files in os.walk("test/error"):
    for f in files:
        if f.endswith(".colgm"):
            TEST_LIST.append(os.path.join(root, f[:-6]))

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
    err_report = de_color(err_report)
    with open(result_text, "r") as fp:
        result = de_color(fp.read())
        if len(result) == 0:
            print("\033[91;1m    FAILED\033[0m empty test result")
            print("\033[91;1m    FAILED\033[0m got:")
            print(err_report)
            continue
        if result not in err_report:
            print("\033[91;1m    FAILED\033[0m expected:")
            print(result)
            print("\033[91;1m    FAILED\033[0m got:")
            print(err_report)
            exit(1)

if os.path.exists("test_error.out.ll"):
    os.remove("test_error.out.ll")
if os.path.exists("test_error.out"):
    os.remove("test_error.out")