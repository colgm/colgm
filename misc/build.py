from util.exec import (
    build_bootstrap_compiler,
    lift_first_version_compiler,
    build_self_host_compiler,
    test_self_lift)
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-self", "--only-self-lift", action="store_true", default=False)
    args = parser.parse_args()

    if args.only_self_lift:
        test_self_lift()
        exit(0)

    # Build bootstrap compiler (written in C++)
    build_bootstrap_compiler()

    # Lift colgm compiler (written in colgm)
    lift_first_version_compiler()

    # Recompile colgm self-host compiler (written in colgm)
    build_self_host_compiler()

    # Test self-host compiler: compiling itself
    test_self_lift()