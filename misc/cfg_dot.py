from util.exec import find_opt, find_dot
import os
import subprocess

output_dir = "cfg.dot"
if not os.path.exists(output_dir):
    os.mkdir(output_dir)
input_file = os.path.abspath("build/colgm_self_host.ll")

os.chdir("cfg.dot")
opt = find_opt()
cmd = opt + " -dot-cfg -cfg-func-name=main " + input_file
if subprocess.run(cmd, shell=True).returncode != 0:
    cmd = opt + " -p=dot-cfg -cfg-func-name=main " + input_file
    if subprocess.run(cmd, shell=True).returncode != 0:
        print("Failed to generate dot file")

dot = find_dot()
for file in os.listdir("."):
    if file.endswith(".dot"):
        cmd = dot + " -Tpng " + file + " -o " + file[:-4] + ".png"
        print("Generating png for " + file)
        subprocess.run(cmd, shell=True)