import subprocess

subprocess.run([
    "valgrind",
    "--leak-check=full",
    "./build/colgm_self_host",
    "--library",
    "src",
    "src/main.colgm",
    "--emit-llvm",
    "-V",
    "-g",
    "-O2"
])