import subprocess
import os

def execute(cmd: list[str]):
    print("Executing: " + " ".join(cmd), flush=True)
    ret = subprocess.run(cmd).returncode
    if ret != 0:
        print("Error: " + " ".join(cmd), flush=True)
        exit(ret)
    return ret

def find_clang():
    suppored_clang_version = []
    for i in range(13, 20):
        suppored_clang_version.append("clang-" + str(i))
        suppored_clang_version.append("clang++-" + str(i))
    suppored_clang_version.append("clang")
    suppored_clang_version.append("clang++")

    for path in os.environ["PATH"].split(":"):
        for clang in suppored_clang_version:
            if os.path.exists(path + "/" + clang):
                return path + "/" + clang
    print("Error: clang not found", flush=True)
    exit(1)

BUILD_DIRECTORY = "build"

if not os.path.exists(BUILD_DIRECTORY):
    os.mkdir(BUILD_DIRECTORY)

# Build bootstrap compiler
os.chdir(BUILD_DIRECTORY)
execute(["cmake", "../bootstrap", "-DCMAKE_BUILD_TYPE=Release"])
execute(["make", "-j6"])
os.chdir("..")

# Build colgm self-host compiler
execute(["./" + BUILD_DIRECTORY + "/colgm", "--library", "src", "src/main.colgm", "-o", "colgm.ll", "--pass-info"])
used_clang = find_clang()
execute([used_clang, "colgm.ll", "-o", "colgm_bootstrapped", "-O2", "-rdynamic", "-lm", "--verbose"])

# Test colgm self-host compiler compiling itself
execute(["./colgm_bootstrapped", "--library", "src", "src/main.colgm", "--verbose"])
