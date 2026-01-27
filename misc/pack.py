import os
import sys
import argparse
import zipfile

NEED_TO_BE_PACKED = [
    "build/colgm",
    "build/colgm_lifted",
    "build/colgm_self_host",
    "bootstrap",
    "doc",
    "src",
    "std"
]

if sys.platform == "win32":
    NEED_TO_BE_PACKED = [
        "cmake-windows-build/Release/colgm.exe",
        "cmake-windows-build/colgm_lifted.exe",
        "cmake-windows-build/colgm_self_host.exe",
        "bootstrap",
        "doc",
        "src",
        "std"
    ]
    if os.path.exists("cmake-windows-build/colgm_lifted.pdb"):
        print("find", "cmake-windows-build/colgm_lifted.pdb")
        NEED_TO_BE_PACKED.append("cmake-windows-build/colgm_lifted.pdb")
    if os.path.exists("cmake-windows-build/colgm_self_host.pdb"):
        print("find", "cmake-windows-build/colgm_self_host.pdb")
        NEED_TO_BE_PACKED.append("cmake-windows-build/colgm_self_host.pdb")

if sys.platform == "darwin":
    if os.path.exists("build/colgm_lifted.dSYM"):
        print("find", "build/colgm_lifted.dSYM")
        NEED_TO_BE_PACKED.append("build/colgm_lifted.dSYM")
    if os.path.exists("build/colgm_self_host.dSYM"):
        print("find", "build/colgm_self_host.dSYM")
        NEED_TO_BE_PACKED.append("build/colgm_self_host.dSYM")

parser = argparse.ArgumentParser()
parser.add_argument("zip_name", type=str, help="zip name")
args = parser.parse_args()

with zipfile.ZipFile(args.zip_name, "w") as zip_file:
    for f in NEED_TO_BE_PACKED:
        print("pack", f)
        if os.path.isdir(f):
            for root, dirs, files in os.walk(f):
                for file in files:
                    print("pack", os.path.join(root, file))
                    zip_file.write(os.path.join(root, file))
            continue
        zip_file.write(f)