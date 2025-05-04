# <img src="doc/logo/colgm.svg" height="45px"/> Colgm Compiler Project

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)
[![nightly-build](https://github.com/colgm/colgm/actions/workflows/release.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/release.yml)

- [__macos-aarch64-nightly-build__](https://github.com/colgm/colgm/releases/tag/macOS_nightly)
- [__linux-x86_64-nightly-build__](https://github.com/colgm/colgm/releases/tag/linux_nightly)
- [__windows-x86_64-nightly-build__ [WIP]](https://github.com/colgm/colgm/releases/tag/windows_nightly)

```rust
use std::io::io;

pub func main() -> i32 {
    io::stdout().out("hello world!").endln();
    return 0;
}
```

## Repo Content

- [bootstrap](./bootstrap/README.md) : bootstrap compiler.
- [src](./src/main.colgm) : self-host compiler.
- [test](./test): test cases.

## Language Guide

See simple language guide in [doc/guide/tutorial.md](./doc/guide/tutorial.md) | [中文版语言指南](./doc/guide/tutorial_zh.md).

## Build and Development

### Requirements

Before building the project, here's the requirements:

- python >= 3.8
- llvm >= 13.0
- clang >= 13.0
- cmake >= 3.21
- zip

### Build

We suggest you to just follow the build script at [misc/build.py](./misc/build.py).

Use this command in top level directory:

```sh
python3 misc/build.py
```

The build script will generate 3 executables in the `build`
directory:

1. `build/colgm`: bootstrap compiler (compiled by gcc/clang)
2. `build/colgm_lifted`: lifted compiler (compiled by `build/colgm`)
3. `build/colgm_selfhost`: self-host compiler (compiled by `build/colgm_lifted`)

If only want to build the self-host compiler __after running the build script once__, you can use this to just build the self-host compiler:

```sh
python3 misc/build.py -self
```

### Test

And use another script to test:

```sh
python3 misc/test.py
```

### Code Style

And for development, you should follow the [code style](./doc/spec/code_style.md).

## Features and Roadmap

1. feature: array type `[<base-type>; 128]`
    - syntax like `var a: [i32; 128] = [1, 2, 3];`
    - syntax like `var a: [i8*; 128] = ["foo", "bar", tmp];`
2. feature: [Tagged Union](./doc/spec/tagged_union.md)
3. feature: [Tuple](./doc/spec/tuple.md)
4. feature: RAII
5. feature: reference type
6. feature: smart pointer
7. feature: std
    - [x] Filesystem API (read, write, join, exists, etc)
    - [x] Datetime utils
    - [ ] String and Unicode Helpers
    - [x] Math Utils
    - [ ] map, filter, reduce, sort, reverse, etc
    - [ ] JSON, TOML, YAML and other formats parsing
    - [x] networking (socket, TCP/UDP Server and Client)
    - [ ] HTTP Utilities
    - [x] OS Utils (exec, env, args, etc)
    - [ ] Typeof, is_type, etc
    - [ ] Assert and Bench
    - [ ] Logging
    - [ ] Deprecation Marker
    - [ ] Regex
    - [ ] Package manager
    - [ ] Docs generator
