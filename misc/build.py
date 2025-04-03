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
BOOTSTRAP_COMPILER = BUILD_DIRECTORY + "/colgm"
SELF_HOST_COMPILER = "./colgm"
SELF_HOST_OUT = "./a.out"
USED_CLANG = find_clang()

if not os.path.exists(BUILD_DIRECTORY):
    os.mkdir(BUILD_DIRECTORY)

# Build bootstrap compiler (written in C++)
os.chdir(BUILD_DIRECTORY)
execute(["cmake", "../bootstrap", "-DCMAKE_BUILD_TYPE=RelWithDebInfo"])
execute(["make", "-j6"])
os.chdir("..")

# Build colgm self-host compiler (written in colgm)
execute(["./" + BOOTSTRAP_COMPILER, "--library", "src", "src/main.colgm", "-o", "colgm.ll", "--pass-info"])
execute([USED_CLANG, "colgm.ll", "-o", SELF_HOST_COMPILER, "-g", "-Oz", "-rdynamic", "-lm", "--verbose"])

# Test colgm self-host compiler compiling itself
execute([SELF_HOST_COMPILER, "--library", "src", "src/main.colgm", "--verbose", "-g", "-Oz"])
execute([SELF_HOST_OUT, "--library", "src", "src/main.colgm", "--verbose", "-g", "-Oz"])
