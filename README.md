# <img src="doc/logo/colgm.svg" height="45px"/> Colgm Compiler Project

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/colgm/colgm)
[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)
[![nightly-build](https://github.com/colgm/colgm/actions/workflows/release.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/release.yml)
[![discord](https://img.shields.io/discord/1369992600853020693?logo=discord&label=Colgm)](https://discord.gg/v8Uta6K8)

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

See simple language guide in:

- [doc/guide/tutorial.md](./doc/guide/tutorial.md)
- [中文版语言指南](./doc/guide/tutorial_zh.md).

See compiler library reference in:

- [doc/guide/library_reference.md](./doc/guide/library_reference.md)

### Usage

Use this command to show help:

```sh
<colgm compiler path> -h
```

#### Example

If you want to compile `test.colgm` with debug mode,
and output the executable file to `test.out`,
use this command:

```sh
<colgm compiler path> test.colgm -g -o test.out
```

#### Optimization

Colgm accepts `-O` option to enable optimization.

- `-O0`: disable optimization
- `-O1`: enable basic optimization
- `-O2`: enable advanced optimization
- `-O3`: enable maximum optimization
- `-Oz`: enable speed optimization
- `-Os`: enable size optimization

Also you could use `-O[x] -g` for release with debug info.

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

If only want to build the bootstrap compiler, you can use this:

```sh
python3 misc/build.py -boot
```

If only want to build the self-host compiler __after running the build script once__, you can use this to just build the self-host compiler:

```sh
python3 misc/build.py -self
```

### Test

And use another script to test:

```sh
python3 misc/test.py
```

Use this script to test tcp/udp utils:

```sh
python3 misc/test_tcp_udp.py
```

To check if any memory leakage, use this script:

```sh
python3 misc/memleak_check.py
```

### Code Style

And for development, you should follow the [code style](./doc/spec/code_style.md).

## Features and Roadmap

1. feature: fuzzy match when field or method name is not found
2. feature: to_string method for struct/enum/tagged union
3. feature: reference type
4. feature: [Tuple](./doc/spec/tuple.md)
5. feature: smart pointer
6. feature: std
    - [x] Filesystem API (read, write, join, exists, etc)
    - [x] Datetime utils
    - [ ] String and Unicode Helpers
    - [x] Math Utils
    - [ ] map, filter, reduce, sort, reverse, etc
    - [ ] JSON, TOML, YAML and other formats parsing
    - [x] networking (socket, TCP/UDP Server and Client)
    - [ ] HTTP Utilities
    - [x] OS Utils (exec, env, args, etc)
    - [x] Assert and Bench
    - [ ] Deprecation Marker
    - [ ] Regex
    - [ ] Package manager
    - [ ] Docs generator
7. SIR refactor
8. constant fold
9. DCE opt
10. CSE opt
