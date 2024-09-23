# <img src="doc/colgm.svg" height="45px"/> Colgm Compiler Project

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)

## Repo Content

- [bootstrap](./bootstrap/README.md) : available now, features not stable these days, but welcome to try.
- [src](./src/README.md) : for bootstrapped compiler, not available now (lexer and parser is ready).

## Bootstrap Compiler

Bootstrap compiler locates at `{repo_path}/bootstrap`, written by C++(`-std=c++17`):

Use these commands to build the bootstrap compiler, use `-DCMAKE_BUILD_TYPE=Debug` if want debug version:

```sh
mkdir build && cd build
cmake ../bootstrap -DCMAKE_BUILD_TYPE=Release && make -j
```

The executable is located as `build/colgm`.

Then use this command to build bootstrapped compiler:

```sh
make colgm.ll
```

The bootstrapped compiler should be `{repo_path}/colgm.ll`.
If having `lli`(llvm jit interpreter), you could try this to
execute bootstrapped new compiler:

```sh
lli colgm.ll main.colgm --library .
```

LLVM jit interpreter maybe untable on some platforms, if lli crashed, try using `clang` to build `out.ll` to executable:

```sh
clang colgm.ll -o colgm
```

## Hello World

Hello world example is available in `bootstrap`:

```rust
extern func puts(v: i8*) -> i32;

pub func main() -> i64 {
    puts("hello world!");
    return 0;
}
```

## Tutorial

See simple tutorial in [bootstrap/README.md](./bootstrap/README.md).

## TODO

1. support foreach/forindex, may not support in bootstrap compiler:
    - forindex loop, container should have `size()` method
    - foreach loop, container should have iterator-like stuff
2. llvm debug info, may not support in bootstrap compiler
3. develop bootstrapped compiler
4. conditional compilation
5. array type like `[i32; 128]`
6. support generics like `struct Vec<T> { ... }`
