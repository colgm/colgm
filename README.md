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
- [src](./src) : self-host compiler.
- [test](./test): test cases.

## Language Guide

See simple language guide in [doc/guide/tutorial.md](./doc/guide/tutorial.md).

## Build and Development

Before building the project, here's the requirements:

- python >= 3.8
- llvm >= 13.0
- cmake >= 3.21
- zip

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

And use another script to test:

```sh
python3 misc/test.py
```

Then it will build the bootstrap compiler and self-host compiler,
and try to build the self-host compiler by itself
(maybe as a test, who knows).

And for development, you should follow the [code style](./doc/spec/code_style.md).

## Features and Roadmap

1. array type `[<base-type>; 128]`
    - syntax like `var a: [i32; 128] = [1, 2, 3];`
    - syntax like `var a: [i8*; 128] = ["foo", "bar", tmp];`
2. feature: [tagged union](./doc/spec/tagged_union.md)
3. feature: [tuple](./doc/spec/tuple.md)
4. feature: RAII
5. feature: reference type
6. feature: smart pointer
