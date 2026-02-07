import os
import sys
import argparse
import zipfile

# now we only pack the following files
# bootstrap files are filtered out
NEED_TO_BE_PACKED = [
    "build/colgm_self_host",
    "doc",
    "src",
    "std",
    "LICENSE",
    "README.md"
]

if sys.platform == "win32":
    NEED_TO_BE_PACKED = [
        "cmake-windows-build/colgm_self_host.exe",
        "doc",
        "src",
        "std",
        "LICENSE",
        "README.md"
    ]
    if os.path.exists("cmake-windows-build/colgm_self_host.pdb"):
        print("find", "cmake-windows-build/colgm_self_host.pdb")
        NEED_TO_BE_PACKED.append("cmake-windows-build/colgm_self_host.pdb")

if sys.platform == "darwin":
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