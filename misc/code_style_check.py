import os
import re

def format_line(line: str) -> str:
    if line.endswith("\n"):
        line = line[0:-1]

    tail_space_count = 0
    while line.endswith(" ") or line.endswith("\t"):
        line = line[0 : -1]
        tail_space_count += 1

    return "#" + line + "." * tail_space_count + "#"

def operator_without_space_in_line(opr: str, line: str) -> bool:
    if opr not in line:
        return False

    if "//" in line or "/*" in line:
        pos = line.find("//")
        if pos == -1:
            pos = line.find("/*")
        if line.find(opr) > pos:
            return False

    # special operator ++
    if opr == "+" and "++" in line:
        return False
    # special operator -, ->
    if opr == "-":
        if "--" in line:
            return False
        pattern = r'-[0-9,x,a-f,A-F,o,e,E,\.]*'
        if re.search(pattern, line):
            return False
        pattern = r'-[a-z,A-Z,_,0-9]*'
        if re.search(pattern, line):
            return False
        if "->" in line:
            pattern = r'[a-z,A-Z,_,0-9]*->[a-z,A-Z,_]*'
            if re.search(pattern, line):
                return False

    if f"operator{opr}" in line:
        return False
    if f" {opr} " in line or f" {opr}\n" in line or f" ={opr} " in line or f" {opr}= " in line:
        return False
    if f" !{opr} " in line or f" {opr}> " in line:
        return False
    if f"\"{opr}" in line or f"{opr}\"" in line:
        return False
    if f"'{opr}'" in line:
        return False
    if f"`{opr}`" in line:
        return False
    return True

def check_tail_space(lines: list[str], file_path):
    count = 0
    for line in lines:
        count += 1
        loc = f"{file_path}:{count}:{len(line)}"
        loc = "{:48}".format(loc)
        if line.endswith(" \n") or line.endswith(" ") or line.endswith(" \t"):
            print(f"[tail-space] {loc} {format_line(line)}")

def check_operator_without_space(lines: list[str], file_path):
    oprs = ["==", "!=", "+", "-", "<=", ">="]
    count = 0
    for line in lines:
        count += 1
        loc = f"{file_path}:{count}:{len(line)}"
        loc = "{:48}".format(loc)
        for opr in oprs:
            if not operator_without_space_in_line(opr, line):
                continue
            print(f"[opr-without-space] {loc} {format_line(line)}")
            break

def check_suffix(file_path):
    suffix = [".colgm", ".c", ".cpp", ".h", ".hpp", ".py", ".md"]
    for s in suffix:
        if file_path.endswith(s):
            return True
    return False

for root, dirs, files in os.walk("."):
    for f in files:
        if not check_suffix(f):
            continue
        file = os.path.join(root, f)
        with open(file, "r") as fp:
            lines = fp.readlines()
        check_tail_space(lines, file)
        check_operator_without_space(lines, file)