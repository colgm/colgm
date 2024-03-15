# Colgm Compiler Project

This is the main repository for Colgm compiler.

## Bootstrap

```sh
cd colgm-bootstrap
make -j4
```

The executable is located as `colgm-bootstrap/colgm`.

In directory `colgm-bootstrap`, use this command:

```sh
make out.ll
```

The generated llvm ir will be generated with filename `out.ll`.
