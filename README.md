# <img src="doc/colgm.svg" height="45px"/> Colgm Compiler Project

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)

## Repo Content

- [bootstrap](./bootstrap/README.md) : available now, features not stable these days, but welcome to try.
- [src](./src/README.md) : for bootstrapped compiler, not available now (lexer and parser is ready).

## Bootstrap Compiler

Bootstrap compiler locates at `{repo_path}/bootstrap`, written by C++(`-std=c++17`):

Use these commands to build the bootstrap compiler, use `-DCMAKE_BUILD_TYPE=Debug` if want debug version:

```sh
cd bootstrap && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release && make -j
```

The executable is located as `build/colgm`.

Then move to the `{repo_path}/src`:

```sh
cd src && make
```

The bootstrapped compiler should be `{repo_path}/src/out.ll`.
If having `lli`(llvm jit interpreter), you could try this to
execute bootstrapped new compiler:

```sh
lli out.ll main.colgm -L .
```

LLVM jit interpreter maybe untable on some platforms, if lli crashed, try using `clang` to build `out.ll` to executable:

```sh
clang out.ll -o colgm
```

Learn more about bootstrap compiler: [Colgm Bootstrap](./bootstrap/README.md).

## Hello World

Hello world example is available in `bootstrap`:

```rust
extern func puts(v: i8*) -> i32;

pub func main() -> i64 {
    puts("hello world!");
    return 0;
}
```

## TODO

1. support foreach/forindex:
    - forindex loop, container should have `size()` method
    - foreach loop, container should have iterator-like stuff
2. public access authorization, use `pub` keyword
3. circular reference detection of structs
4. llvm debug info
5. develop bootstrapped compiler
