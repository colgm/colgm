# <img src="../doc/colgm.svg" height="50px"/> Colgm Bootstrap Compiler

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)

This directory stores the source file of colgm bootstrap compiler.

![llvm?](../doc/jpg/llvm-bootstrap.jpg)

## Build

In the top level directory, use these commands:

```sh
cd bootstrap
mkdir build && cd build && cmake ..
make -j
```

## Usage

Use this command to get usage:

```sh
./build/colgm -h
```

## Syntax

### Arithmetic Operator

Colgm allows these arithmetic operators:

```typescript
a + b;
a - b;
a * b;
a / b;
a % b;
```

### Bitwise Operator

Colgm now supports these bitwise operators:

```typescript
a | b;
a ^ b;
a & b;
```

### Logical Operator

Support basic logical operators: `==` `!=` `<` `<=` `>` `>=`.

Logical operators for `and`/`or` all have two types:

```typescript
1==1 and 2==2 && 3==3;
1==1 or 2==2 || 3==3;
```

### Definition

Colgm allows two kinds of definition.

```typescript
var variable_name: type = expression; # with type
var variable_name = expression; # without type
```

### Assignment

Colgm allows these assignment operators:

```typescript
a += b;
a -= b;
a *= b;
a /= b;
a %= b;
a |= b;
a ^= b;
a &= b;
```

### Control Flow

```typescript
# loop
while (1 == 1) {
    ...
    continue;
    break;
}

# condition
if (1 == 1) {
    ...
} elsif (1 == 1) {
    ...
} else {
    ...
}
```

## Type Conversion

A bit like rust, using `=>` instead of `as`:

```rust
func main() -> i32 {
    return 0 => i32;
}
```
