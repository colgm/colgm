import os

def check_file(file_path):
    with open(file_path, "r") as fp:
        lines = fp.readlines()
        count = 0
        for line in lines:
            count += 1
            loc = f"{file_path}:{count}:{len(line)}"
            loc = "{:48}".format(loc)
            if line.endswith(" \n"):
                print(f"{loc} #{line[0:-1].replace(' ', '.')}#")
            elif line.endswith(" "):
                print(f"{loc} #{line.replace(' ', '.')}#")

for root, dirs, files in os.walk("."):
    for f in files:
        if f.endswith(".colgm") or f.endswith(".cpp") or f.endswith(".h"):
            check_file(os.path.join(root, f))