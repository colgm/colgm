from threading import Thread
import subprocess
import time
import os
import sys
import shutil

COMPILER = "build/colgm_self_host"
if sys.platform == "win32":
    COMPILER = "cmake-windows-build\\colgm_self_host.exe"

def check_required():
    if not os.path.exists(COMPILER):
        print("Error: build/colgm_self_host not found", flush=True)
        exit(1)
    if not os.path.exists("example/socket/udp_example.colgm"):
        print("Error: example/socket/udp_example.colgm not found", flush=True)
        exit(1)
    if not os.path.exists("example/socket/tcp_example.colgm"):
        print("Error: example/socket/tcp_example.colgm not found", flush=True)
        exit(1)

def compile_tcp():
    subprocess.run([
        COMPILER,
        "--library", "src",
        "example/socket/tcp_example.colgm",
        "-g",
        "-o", "tcp_udp_test.out"
    ])

def compile_udp():
    subprocess.run([
        COMPILER,
        "--library", "src",
        "example/socket/udp_example.colgm",
        "-g",
        "-o", "tcp_udp_test.out"
    ])

def clear():
    if os.path.exists("tcp_udp_test.out"):
        os.remove("tcp_udp_test.out")
    if os.path.exists("tcp_udp_test.out.ll"):
        os.remove("tcp_udp_test.out.ll")
    if os.path.exists("tcp_udp_test.out.dSYM"):
        shutil.rmtree("tcp_udp_test.out.dSYM")
    if os.path.exists("tcp_udp_test.ilk"):
        os.remove("tcp_udp_test.ilk")
    if os.path.exists("tcp_udp_test.pdb"):
        os.remove("tcp_udp_test.pdb")

ERROR_RETURN = False

def run_server():
    if subprocess.run(["./tcp_udp_test.out", "server"]).returncode != 0:
        global ERROR_RETURN
        ERROR_RETURN = True

def run_client():
    if subprocess.run(["./tcp_udp_test.out", "client"]).returncode != 0:
        global ERROR_RETURN
        ERROR_RETURN = True

def test_tcp():
    compile_tcp()
    server = Thread(target=run_server)
    client = Thread(target=run_client)

    server.start()
    time.sleep(1)
    client.start()

    server.join()
    client.join()

    if ERROR_RETURN:
        print("Error: test_tcp failed", flush=True)
        exit(1)

def test_udp():
    compile_udp()
    server = Thread(target=run_server)
    client = Thread(target=run_client)

    server.start()
    time.sleep(1)
    client.start()

    server.join()
    client.join()

    if ERROR_RETURN:
        print("Error: test_udp failed", flush=True)
        exit(1)

if __name__ == "__main__":
    check_required()
    test_tcp()
    test_udp()
    clear()
