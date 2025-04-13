# Colgm Language Guide

![just a picture](../jpg/llvm-bootstrap.jpg)

## Content

- [Literal](#literal)
  - [Default Literal Type](#default-literal-type)
  - [Auto Type Cast](#auto-type-cast)
- [Operators](#operators)
  - [Arithmetic Operator](#arithmetic-operator)
  - [Bitwise Operator](#bitwise-operator)
  - [Logical Operator](#logical-operator)
- [Definition](#definition)
- [Assignment](#assignment)
- [Control Flow](#control-flow)
  - [While Loop](#while-loop)
  - [Condition/Branch](#conditionbranch)
  - [Match](#match)
  - [For Loop](#for-loop)
  - [Foreach/ForIndex Loop [WIP]](#foreachforindex-loop-wip)
- [Type Conversion](#type-conversion)
- [Structure](#structure)
  - [Struct Initializer](#struct-initializer)
- [Implementation](#implementation)
- [Enumeration](#enumeration)
- [Conditional Compilation](#conditional-compilation)
- [Tagged Union](../spec/tagged_union.md)
- [Tuple](../spec/tuple.md)
- [Compiler Library Reference](./library_reference.md)

## Literal

### Default Literal Type

Literals in colgm all have default type.

- integer: `i64`
  - integer beginning with `0x` is treated as hexadecimal integer, type `u64`
  - integer beginning with `0o` is treated as octal integer, type `u64`
- float: `f64`
- boolean: `bool`, with two keyword `true` and `false`
- constant string: `i8*`
- array: const pointer of its base type
- nil: special pointer `nil` with type `i8*`

```rust
func main() -> i32 {
    var a = 1;              // i64
    var b = 2.0;            // f64
    var c = "hello world!"; // i8*
    var d = 'd';            // i8
    var e = [i8; 128];      // const i8*
    var f = true;           // bool
    var g = nil;            // i8*

    var h = 0x7fffffff;     // u64
    var i = 0o777;          // u64
    return 0;
}
```

### Auto Type Cast

A feature named auto type cast is supported in colgm.
Be ware that this feature only works on literals.
For example although `1` is a integer literal with type `i64`,
it will be casted to `i32` when used in these cases:

1. definition with type declaration:

    ```rust
    var a: i32 = 1;    // i64 to i32
    var b: i64* = nil; // i8* to i64*
    ```

2. function call:

    ```rust
    pub func foo(a: i32, b: i32) -> i32 {
        return a + b;
    }

    func main() -> i32 {
        return foo(1, 2); // 1 i64 => i32, 2 i64 => i32
    }
    ```

3. assignment: `a = 1;` for a is `i16`, the `1` will be converted to `i16`
4. calculation: `a + b * 16 - d / e` for variables' type are all `i32`, the `16` will be converted to `i32`
5. return statement: `return 1;` for return type is `i32`, the `1` will be converted to `i32`

Total examples:

```rust
pub func test(arg: u32) -> u8 {
    return arg => u8;
}

pub func main() -> i32 {
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
    test(42);            // i64(42) => u32
    return 0;            // i64       => i32
}
```

Also available when initializing a struct:

```rust
struct Example {
    a: u64,
    next: Example*
}

func main() -> i32 {
    var s = Example {
        a: 1,     // i64(1)  => u64
        next: nil // i8*(nil) => Example*
    };            //
    return 0;     // i64(0)  => i32
}
```

## Operators

### Arithmetic Operator

Colgm allows these arithmetic operators:

```rust
a + b; // add
a - b; // sub
a * b; // mul
a / b; // div
a % b; // mod, only used on integer type
```

### Bitwise Operator

Colgm supports these bitwise operators, only integer type is supported:

```rust
a | b; // bit or
a ^ b; // bit xor
a & b; // bit and
```

### Logical Operator

Colgm supports basic logical operators: `==` `!=` `<` `<=` `>` `>=`.
Only boolean type is allowed.

Logical operators for `and`/`or` all have two syntax:

```rust
1==1 and 2==2 && 3==3;
//   ^^^      ^^ same logical and
1==1 or 2==2 || 3==3;
//   ^^      ^^  same logical or
```

## Definition

Colgm allows two kinds of definitions.
Definitions without type declaration will use type inference,
so make sure the init value's type is what you want.

```rust
var variable: type = expression; // with type
var variable = expression;       // without type
```

## Assignment

Colgm allows these assignment operators:

```rust
a += b; // add
a -= b; // sub
a *= b; // mul
a /= b; // div
a %= b; // mod, only used on integer type
a |= b; // bit or, only used on integer type
a ^= b; // bit xor, only used on integer type
a &= b; // bit and, only used on integer type
```

## Control Flow

### While Loop

While loops are the same in C language.

```rust
while (1 == 1) {
    ...
    continue;
    break;
}
```

### Condition/Branch

Conditionals are the same in C language.
The syntax is the same as C language,
except a keyword `elsif`, which shares the same use as `else if`.

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

Colgm supports `match` keyword.
Match statements only accept enum types.
The default branch is `_`

```rust
match (variable) {
    EnumType::a => ...;
    EnumType::b => { ... }
    _ => { ... }
}
```

### For Loop

```rust
for (var i = 0; i < 10; i += 1)  {}

// same as below:
//
// var i = 0;
// while (i < 10) {
//     i += 1;
// }
```

### Foreach/ForIndex Loop [WIP]

Work in progress.

```rust
foreach (var i; container) {}
// same as below:
//
// var tmp_i = container.iter();
// while (!tmp_i.is_end()) {
//     var i = tmp_i;
//     ...
//     tmp_i = tmp_i.next();
// }
```

```rust
forindex (var i; container) {}
// same as below:
//
// var tmp_i = container.iter();
// while (!tmp_i.is_end()) {
//     var i = tmp_i.index();
//     tmp_i = tmp_i.next();
// }
```

## Type Conversion

For auto type cast, see [Auto Type Cast](#auto-type-cast).

A bit like rust, using `=>` instead of `as`:

```rust
use std::libc::malloc;

func main() -> i32 {
    var res1 = 0 => i32;
    var res2: i32 = 0; // same as above, using auto type cast
    var ptr1 = malloc(1024) => i16*;
    return res1 + res2;
}
```

## Structure

Structures are the same in C language.
__Now RAII is not supported__.
So be careful when moving a struct to another struct,
this will do __shallow__ copy.

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

Colgm supports struct initializer,
and some member fields can be omitted.
But be careful for not initializing all fields.
If a field is used without initialization,
undefined behavior will occur.

Now colgm set the default data of struct to llvm `zeroinitializer`.

```rust
func test() {
    var s1 = StructName {};
    var s2 = StructName { field1: 1 };
    var s3 = StructName { field1: 1, field2: 2 };
}
```

## Implementation

Colgm supports `impl` keyword.
For methods, the first parameter is `self`,
which is a pointer to the struct.
For static methods, the first parameter should not be `self`.

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

## Enumeration

Enumerations are the same of `enum class` in C++ language.
By default, the first member of enum is `0`,
and the next member is `1`, then `2`, and so on.
But you can specify the value of each member.

```rust
enum EnumExample {
    kind_a,
    kind_b,
    kind_c
}

enum EnumSpecified {
    kind_a = 0x100,
    kind_b = 0x200,
    kind_c = 0x300
}
```

## Conditional Compilation

Conditional compilation is a feature
that allows you to compile code based on certain conditions.
For example cross platform issues can be solved by conditional compilation.
Now only support `enable_if`, later more conditions will be supported.

```rust
#[enable_if(target_os="linux")]
pub enum flag {
    O_RDONLY = 0x000,
    O_WRONLY = 0x001,
    O_RDWR   = 0x002,
    O_CREAT  = 0x040,
    O_EXCL   = 0x080,
    O_TRUNC  = 0x200,
    O_APPEND = 0x400
}
```
