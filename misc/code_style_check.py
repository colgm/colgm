import os

def format_line(line: str) -> str:
    if line.endswith("\n"):
        line = line[0:-1]
    return "#" + line.replace(" ", ".") + "#"

def operator_without_space_in_line(opr: str, line: str) -> bool:
    if opr not in line:
        return False
    if f" {opr} " in line or f"={opr}" in line or f"{opr}=" in line:
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
            print(f"{loc} {format_line(line)}")

def check_operator_without_space(lines: list[str], file_path):
    oprs = ["==", "!="]
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