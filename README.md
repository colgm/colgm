# <img src="doc/colgm.svg" height="50px"/> Colgm Compiler Project

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)

This is the main repository for Colgm compiler.

![llvm?](./doc/jpg/llvm-maybe.jpg)

## Repo Content

- [Bootstrap](./bootstrap/README.md) -> bootstrap
  - still under development
- [Bootstrapped [WIP]](./src/README.md) -> src
  - for bootstrap test, not useful

## Bootstrapping Compiler

Bootstrap compiler locates at `{repo_path}/bootstrap`,
written by C++(`-std=c++17`):

Use these commands to build the bootstrap compiler:

```sh
cd bootstrap && mkdir build && cd build && cmake .. && make -j
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

Learn more about bootstrap compiler: [Colgm Bootstrap](./bootstrap/README.md).

## Hello World

Hello world example is available in `bootstrap`:

```rust
func puts(v: i8*) -> i32;

func main() -> i64 {
    puts("hello world!");
    return 0;
}
```

## TODO

1. support for/foreach/forindex:
    - for loop
    - forindex loop, container should have `size()` method
    - foreach loop, container should have iterator-like stuff
2. llvm debug info
3. develop bootstrapped compiler
