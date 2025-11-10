import subprocess
import os
import sys
import shutil

def is_windows() -> bool:
    return sys.platform == "win32"

def execute(cmd: list[str]) -> int:
    print("Executing: " + " ".join(cmd), flush=True)
    ret = subprocess.run(cmd).returncode
    if ret != 0:
        print("Error: " + " ".join(cmd), flush=True)
        exit(ret)
    return ret

def get_path() -> list[str]:
    return os.environ["PATH"].split(";" if is_windows() else ":")

def find_clang() -> str:
    suffix = ".exe" if is_windows() else ""
    suppored_clang_version = []
    for i in range(13, 20):
        suppored_clang_version.append("clang-" + str(i) + suffix)
        suppored_clang_version.append("clang++-" + str(i) + suffix)
    suppored_clang_version.append("clang" + suffix)
    suppored_clang_version.append("clang++" + suffix)

    for path in get_path():
        for clang in suppored_clang_version:
            if os.path.exists(os.path.join(path, clang)):
                return os.path.join(path, clang)
    print("Error: clang not found", flush=True)
    exit(1)

def find_cmake():
    suffix = ".exe" if is_windows() else ""
    for path in get_path():
        possible_path = os.path.join(path, "cmake" + suffix)
        if os.path.exists(possible_path):
            print("CMake: found", possible_path, flush=True)
            return possible_path
    print("Error: cmake not found", flush=True)
    exit(1)

def find_opt():
    suffix = ".exe" if is_windows() else ""
    suppored_opt_version = []
    for i in range(13, 20):
        suppored_opt_version.append("opt-" + str(i) + suffix)
    suppored_opt_version.append("opt" + suffix)

    for path in get_path():
        for opt in suppored_opt_version:
            if os.path.exists(os.path.join(path, opt)):
                return os.path.join(path, opt)
    print("Error: opt not found", flush=True)
    exit(1)

def find_dot():
    suffix = ".exe" if is_windows() else ""
    dot = "dot" + suffix

    for path in get_path():
        if os.path.exists(os.path.join(path, dot)):
            return os.path.join(path, dot)
    print("Error: dot not found", flush=True)
    exit(1)

BUILD_DIRECTORY = "build"
if is_windows():
    BUILD_DIRECTORY = "cmake-windows-build"

BOOTSTRAP_COMPILER = os.path.join(BUILD_DIRECTORY, "colgm")           # compiler written in C++
if is_windows():
    BOOTSTRAP_COMPILER  = os.path.join(BUILD_DIRECTORY, "Release\colgm.exe")

LIFTED_COMPILER    = os.path.join(BUILD_DIRECTORY, "colgm_lifted")    # compiler written in colgm, but lifted by bootstrap compiler
SELF_HOST_COMPILER = os.path.join(BUILD_DIRECTORY, "colgm_self_host") # compiler written in colgm, compiled by lifted compiler
                                                                      # at this time we have the self host compiler
if is_windows():
    LIFTED_COMPILER    += ".exe"
    SELF_HOST_COMPILER += ".exe"
USED_CLANG = find_clang()

def get_used_cpu_count():
    count = os.cpu_count()
    if count is not None and count > 1:
        return count // 2
    return 1

def build_bootstrap_compiler():
    cmake = find_cmake()

    if not os.path.exists(BUILD_DIRECTORY):
        os.mkdir(BUILD_DIRECTORY)
    os.chdir(BUILD_DIRECTORY)
    execute([
        cmake,
        "../bootstrap",
        "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
    ])
    if is_windows():
        execute([
            "MSBuild.exe",
            "colgm.sln",
            "/p:Configuration=Release",
            "/p:Platform=x64",
            "-maxcpucount:{}".format(get_used_cpu_count())
        ])
    else:
        execute(["make", "-j{}".format(get_used_cpu_count())])
    os.chdir("..")

def lift_first_version_compiler():
    if not os.path.exists(BUILD_DIRECTORY):
        os.mkdir(BUILD_DIRECTORY)
    lifted_ll = os.path.join(BUILD_DIRECTORY, "colgm_lifted.ll")
    execute([
        BOOTSTRAP_COMPILER,
        "--library", "src",
        "src/main.colgm",
        "-o",
        lifted_ll,
        "--pass-info"
    ])
    if is_windows():
        execute([
            USED_CLANG,
            lifted_ll,
            "-o", LIFTED_COMPILER,
            "-g", "-Oz",
            "-Xclang", "-triple=x86_64-pc-windows-msvc",
            "-ldbghelp", "-lws2_32", "-lkernel32"
        ])
    else:
        execute([
            USED_CLANG,
            lifted_ll,
            "-o", LIFTED_COMPILER,
            "-g", "-Oz",
            "-rdynamic", "-lm"
        ])

def build_self_host_compiler():
    if not os.path.exists(BUILD_DIRECTORY):
        os.mkdir(BUILD_DIRECTORY)
    execute([
        LIFTED_COMPILER,
        "--library", "src",
        "src/main.colgm",
        "--verbose", "-g", "-Oz",
        "-o", SELF_HOST_COMPILER
    ])

def test_self_lift():
    if is_windows():
        # windows does not allow to overwrite executable while it is running
        SELF_HOST_COMPILER_TEMP = os.path.join(BUILD_DIRECTORY, "colgm_self_host_temp.exe")
        execute([
            SELF_HOST_COMPILER,
            "--library", "src",
            "src/main.colgm",
            "--verbose", "-g", "-Oz",
            "-o",
            SELF_HOST_COMPILER_TEMP
        ])
        shutil.move(SELF_HOST_COMPILER_TEMP, SELF_HOST_COMPILER)
        return
    execute([
        SELF_HOST_COMPILER,
        "--library", "src",
        "src/main.colgm",
        "--verbose", "-g", "-Oz",
        "-o",
        SELF_HOST_COMPILER
    ])
