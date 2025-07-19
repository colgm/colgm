from util.exec import execute
import os
import sys
import argparse

NEED_TO_BE_PACKED = [
    "build/colgm",
    "build/colgm_lifted",
    "build/colgm_self_host",
    "bootstrap",
    "doc",
    "src"
]

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

for f in NEED_TO_BE_PACKED:
    execute(["zip", "-r", args.zip_name, f])