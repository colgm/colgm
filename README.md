# Colgm Compiler Project

This is the main repository for Colgm compiler.

## Content

- [colgm bootstrap](./colgm-bootstrap/README.md) -> colgm-bootstrap

- [colgm self-hosted](./src/README.md) -> src

## Bootstrap

Use these commands to build the bootstrap compiler:

```sh
mkdir build
cd build
cmake ../colgm-bootstrap
make -j
```

The executable is located as `build/colgm`.

In directory `colgm-bootstrap`, use this command:

```sh
make out.ll
```

The generated llvm ir will be generated with filename `out.ll`.

## Hello World

Hello world example is available in `colgm-bootstrap`:

```rust
func puts(v: i8*) -> i32;

func main() -> i64 {
    puts("hello world!");
    return 0;
}
```
