# <img src="../doc/colgm.svg" height="45px"/> Colgm Bootstrap Compiler

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)
[![nightly-build](https://github.com/colgm/colgm/actions/workflows/release.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/release.yml)

This directory stores the source file of colgm bootstrap compiler.

## Build

In the top level directory, use these commands:

```sh
mkdir build && cd build && cmake ../bootstrap -DCMAKE_BUILD_TYPE=Release && make -j
```

## Usage

Use this command to get usage:

```sh
./build/colgm -h
```

## Literal

```rust
func main() -> i32 {
    var a = 1;              // i64
    var b = 2.0;            // f64
    var c = "hello world!"; // i8*
    var d = 'd';            // i8
    var e = [i8; 128];      // const i8*
    var f = true;           // bool
    return 0;
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

```rust
var variable_name: type = expression; // with type
var variable_name = expression; // without type
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

### While Loop

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

### For Loop

```rust
for (var i = 0; i < 10; i += 1)  {}
=>
var i = 0;
while (i < 10) {
    i += 1;
}
```

### Foreach/ForIndex Loop [WIP]

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
    var res1 = 0 => i32;
    var res2: i32 = 0; // same as above
    return res1 + res2;
}
```

### Automatic Type Cast

Number literals and `nil` will be casted to the target type automatically.

```rust
func main() -> i32 {
    var a: i32 = 1;      // i64(1)    => i32
    var b: i64* = nil;   // i8*(nil)  => i64*
    if (a < 10) {        // i64(10)   => i32
        return 1;        // i64(1)    => i32
    } else if (a != 1) { // i64(1)    => i32
        return -1;       // i64(-1)   => i32
    }                    //
    var c = 'a';         //
    c += 1;              // i64(1)    => i8
                         //
    var d = 1.0;         // f64
    var e: f32 = 2.71;   // f64(2.71) => f32
                         //
    return 0;            // i64       => i32
}
```

Also available when initializing a struct or calling a function:

```rust
struct Example {
    a: u64,
    next: Example*
}

pub func test(arg: u32) -> u8 {
    return arg => u8;
}

func main() -> i32 {
    var s = Example {
        a: 1,     // i64(1)  => u64
        next: nil // i8*(nil) => Example*
    };            //
    test(42);     // i64(42) => u32
    return 0;     // i64(0)  => i32
}
```

## Struct Definition

```rust
struct StructName {
    field1: i64,
    field2: i64
}

// using generics
struct StructWithGeneric<T> {
    __size: i64,
    __data: T*
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

// impl generics
impl StructWithGeneric<T> {
    ...
}
```

## Enum Definition

```rust
enum EnumExample {
    kind_a,
    kind_b,
    kind_c
}

enum EnumSpecified {
    kind_a = 0x1,
    kind_b = 0x2,
    kind_c = 0x3
}
```
