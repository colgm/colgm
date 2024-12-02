# <img src="../doc/colgm.svg" height="50px"/> Self-Host Colgm Compiler

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)
[![nightly-build](https://github.com/colgm/colgm/actions/workflows/release.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/release.yml)

Work in progress. Lexer and parser are done.

## Build Bootstrap Compiler

First you need bootstrap compiler, then use this command:

```bash
[path of the compiler] main.colgm --library . -o colgm.ll
```

And `colgm.ll` is generated, try `lli` or `clang`.

## Build Itself (Self-Host Compiler)

for `lli` (LLVM JIT compiler):

```bash
lli colgm.ll main.colgm --library . -o out.ll
```

for `clang` (suggested):

```bash
clang colgm.ll -o colgm
./colgm main.colgm --library . -o out.ll
```
