# Colgm Compiler Project

<img src="./doc/png/logo-small.png" style="width:200px"></img>

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)

This is the main repository for Colgm compiler.

## Repo Content

- [Bootstrap [WIP]](./bootstrap/README.md) -> bootstrap
- [Self-Hosted [WIP & unavailable now]](./src/README.md) -> src

## Bootstrap Compiler

Use these commands to build the bootstrap compiler:

```sh
cd bootstrap
mkdir build
cd build
cmake ..
make -j
```

The executable is located as `build/colgm`.

In directory `bootstrap`, use this command:

```sh
make out.ll
```

The generated llvm ir will be generated with filename `out.ll`.

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
