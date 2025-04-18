import subprocess
import os

def execute(cmd: list[str]) -> int:
    print("Executing: " + " ".join(cmd), flush=True)
    ret = subprocess.run(cmd).returncode
    if ret != 0:
        print("Error: " + " ".join(cmd), flush=True)
        exit(ret)
    return ret

def find_clang() -> str:
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
BOOTSTRAP_COMPILER = "build/colgm"        # compiler written in C++
LIFTED_COMPILER = "build/colgm_lifted" # compiler written in colgm, but lifted by bootstrap compiler
SELF_HOST_COMPILER = "build/colgm_self_host"   # compiler written in colgm, compiled by lifted compiler
                                          # at this time we have the self host compiler
USED_CLANG = find_clang()

if not os.path.exists(BUILD_DIRECTORY):
    os.mkdir(BUILD_DIRECTORY)

# Build bootstrap compiler (written in C++)
os.chdir(BUILD_DIRECTORY)
execute(["cmake", "../bootstrap", "-DCMAKE_BUILD_TYPE=RelWithDebInfo"])
execute(["make", "-j6"])
os.chdir("..")

# Lift colgm compiler (written in colgm)
execute([BOOTSTRAP_COMPILER, "--library", "src", "src/main.colgm", "-o", "build/colgm_bootstrap_init.ll", "--pass-info"])
execute([USED_CLANG, "build/colgm_bootstrap_init.ll", "-o", LIFTED_COMPILER, "-g", "-Oz", "-rdynamic", "-lm", "--verbose"])

# Recompile colgm self-host compiler (written in colgm)
execute([LIFTED_COMPILER, "--library", "src", "src/main.colgm", "--verbose", "-g", "-Oz", "-o", SELF_HOST_COMPILER])
# Test self-host compiler: compiling itself
execute([SELF_HOST_COMPILER, "--library", "src", "src/main.colgm", "--verbose", "-g", "-Oz", "-o", "a.out"])
