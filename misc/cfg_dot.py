from util.exec import find_opt
import subprocess

opt = find_opt()
cmd = opt + " -dot-cfg build/colgm_self_host.ll -cfg-func-name=main"
subprocess.run(cmd, shell=True)