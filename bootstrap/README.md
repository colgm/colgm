# <img src="../doc/colgm.svg" height="50px"/> Colgm Bootstrap Compiler

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)

This directory stores the source file of colgm bootstrap compiler.

## Build

In the top level directory, use these commands:

```sh
cd bootstrap && mkdir build && cd build && cmake .. && make -j
```

## Usage

Use this command to get usage:

```sh
./build/colgm -h
```

## Literal

```typescript
func main() -> i32 {
    var a = 1;              # i64
    var b = 2.0;            # f64
    var c = "hello world!"; # i8*
    var d = 'd';            # i8
    var e = [i8; 128];      # i8*
    var f = true;           # bool
    return 0 => i32;
}
```

## Operators

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

## Definition

Colgm allows two kinds of definition.

```typescript
var variable_name: type = expression; # with type
var variable_name = expression; # without type
```

## Assignment

Colgm allows these assignment operators:

```rust
a += b;
a -= b;
a *= b;
a /= b;
a %= b;
a |= b;
a ^= b;
a &= b;
```

## Control Flow

### While

while loop is directly translated to `mir_loop` in colgm mir.

```rust
while (1 == 1) {
    ...
    continue;
    break;
}
```

### Condition/Branch

```rust
if (1 == 1) {
    ...
} elsif (1 == 1) {
    ...
} else if (1 == 1) {
    ...
} else {
    ...
}
```

### Match

```rust
match (variable) {
    EnumType::a => ...
    EnumType::b => ...
}
```

### For

```rust
for (var i = 0; i < 10; i += 1)  {}
=>
var i = 0;
while (i < 10) {
    i += 1;
}
```

### Foreach/ForIndex [WIP]

```rust
foreach (var i; container) {}
=>
var tmp_i = container.begin();
while (tmp_i != container.end()) {
    var i = tmp_i.value();
    tmp_i = tmp_i.next();
}
```

```rust
forindex (var i; container) {}
=>
var tmp_i = 0;
while (tmp_i < container.size()) {
    var i = tmp_i;
    tmp_i += 1;
}
```

## Type Conversion

A bit like rust, using `=>` instead of `as`:

```rust
func main() -> i32 {
    return 0 => i32;
}
```

## Struct Definition

```rust
struct StructName {
    field1: i64,
    field2: i64
}
```

### Struct Initializer

```rust
func test() {
    var s1 = StructName {};
    var s2 = StructName { field1: 1 };
    var s3 = StructName { field1: 1, field2: 2 };
}
```

## Implementation

```rust
impl StructName {
    func method(self) -> type {
        ...
        return ...;
    }
    func static_method() -> type {
        ...
        return ...;
    }
}
```

## Enum Definition

```rust
enum EnumName {
    a,
    b,
    c
}

enum EnumName {
    a = 0x1,
    b = 0x2,
    c = 0x3
}
```
