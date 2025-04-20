# <img src="../doc/logo/colgm.svg" height="45px"/> Colgm Bootstrap Compiler

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)
[![nightly-build](https://github.com/colgm/colgm/actions/workflows/release.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/release.yml)

This directory stores the source file of colgm bootstrap compiler.

## Build

In the top level directory, use these commands:

```sh
mkdir -p build && cd build
cmake ../bootstrap -DCMAKE_BUILD_TYPE=Release && make -j6
```

## Build Self-Host Compiler

On top level directory,
use this command to get usage:

```sh
./build/colgm -h
```

On top level directory,
to build the first version of self-host compiler,
use this command:

```sh
./build/colgm --library src src/main.colgm -o colgm.ll --pass-info
<clang> colgm.ll -o colgm -g -Oz -rdynamic -lm --verbose
```
