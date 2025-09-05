# Colgm 语言指南

![只是一张图片](../jpg/llvm-bootstrap.jpg)

## 目录

- [字面量](#字面量)
  - [默认字面量类型](#默认字面量类型)
  - [自动类型转换](#自动类型转换)
  - [数组初始化](#数组初始化)
- [运算符](#运算符)
  - [算术运算符](#算术运算符)
  - [位运算符](#位运算符)
  - [逻辑运算符](#逻辑运算符)
- [定义](#定义)
  - [变量名覆盖](#变量名覆盖)
- [赋值](#赋值)
- [控制流](#控制流)
  - [While 循环](#while-循环)
  - [条件/分支](#条件分支)
  - [匹配](#匹配)
  - [For 循环](#for-循环)
  - [Foreach/ForIndex 循环](#foreachforindex-循环)
- [引用](../spec/ref_type.md)
- [类型转换](#类型转换)
- [结构体](#结构体)
  - [结构体初始化器](#结构体初始化器)
- [实现](#实现)
- [枚举](#枚举)
- [编译器内建函数](#编译器内建函数)
- [条件编译](#条件编译)
- [标记联合](../spec/tagged_union.md)
- [Defer](../spec/defer.md)
- [编译器库参考](./library_reference.md)

## 字面量

### 默认字面量类型

Colgm 中的字面量都有默认类型。

- 整数：`i64`
  - 以 `0x` 开头的整数被视为十六进制整数，类型为 `u64`
  - 以 `0o` 开头的整数被视为八进制整数，类型为 `u64`
- 浮点数：`f64`
- 布尔值：`bool`，有两个关键字 `true` 和 `false`
- 常量字符串：`const i8*`
- 数组：其基本类型的指针
  - 具有此类型的变量是不可变的
  - 如果基本类型不是常量，则数组中的元素是可变的
- nil：特殊指针 `nil`，类型为 `i8*`

```rust
func main() -> i32 {
    var a = 1;              // i64
    var b = 2.0;            // f64
    var c = "hello world!"; // const i8*
    var d = 'd';            // i8
    var e = [i8; 128];      // (array) i8*
    var f = true;           // bool
    var g = nil;            // i8*

    var h = 0x7fffffff;     // u64
    var i = 0o777;          // u64
    return 0;
}
```

### 自动类型转换

Colgm 支持一种名为自动类型转换的特性。
请注意，此特性仅适用于字面量。
例如，虽然 `1` 是类型为 `i64` 的整数字面量，
但在以下情况下它将被转换为 `i32`：

1. 带有类型声明的定义：

    ```rust
    var a: i32 = 1;    // i64 转 i32
    var b: i64* = nil; // i8* 转 i64*
    ```

2. 函数调用：

    ```rust
    pub func foo(a: i32, b: i32) -> i32 {
        return a + b;
    }

    func main() -> i32 {
        return foo(1, 2); // 1 i64 => i32, 2 i64 => i32
    }
    ```

3. 赋值：对于类型为 `i16` 的 `a`，`a = 1;` 中的 `1` 将被转换为 `i16`
4. 计算：对于类型全为 `i32` 的变量，`a + b * 16 - d / e` 中的 `16` 将被转换为 `i32`
5. 返回语句：对于返回类型为 `i32` 的函数，`return 1;` 中的 `1` 将被转换为 `i32`

完整示例：

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

在初始化结构体时也可用：

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

### 数组初始化

Colgm 支持数组的写法。数组可以通过多种方法来初始化。
但是必须要注意，空数组的初始化必须声明类型。

```rust
func main() -> i32 {
    var a = [1, 2, 3]; // [i64; 3]
    //       ^       => 基础类型由第一个元素的类型决定
    //       ^^^^^^^ => 数组大小由元素数量决定

    // [i32; 1024] 并且第一个元素是 0
    var b: [i32; 1024] = [0 => i32];

    // error: 无法推导数组类型
    var c = [];
    return 0;
}
```

## 运算符

### 算术运算符

Colgm 允许以下算术运算符：

```rust
a + b; // 加
a - b; // 减
a * b; // 乘
a / b; // 除
a % b; // 模，仅用于整数类型
```

### 位运算符

Colgm 支持以下位运算符，仅支持整数类型：

```rust
a | b; // 位或
a ^ b; // 位异或
a & b; // 位与
```

### 逻辑运算符

Colgm 支持基本逻辑运算符：`==` `!=` `<` `<=` `>` `>=`。
仅允许布尔类型。

`and`/`or` 的逻辑运算符都有两种语法：

```rust
1==1 and 2==2 && 3==3;
//   ^^^      ^^ 相同的逻辑与
1==1 or 2==2 || 3==3;
//   ^^      ^^  相同的逻辑或
```

## 定义

Colgm 允许两种定义方式。
没有类型声明的定义将使用类型推断，
所以确保初始值的类型是您想要的。

```rust
var variable: type = expression; // 带类型
var variable = expression;       // 不带类型
```

### 变量名覆盖

Colgm 不允许变量名覆盖。

```rust
var a: i32 = 1;
if (1 == 1) {
    var a: f64 = 1;
    //  ^^^^^^ 变量名覆盖(name shadowing)
}
```

但是下面这个样例是允许的。

```rust
if (1 == 1) {
    var b = 1;
} else {
    var b = 1.0;
}
```

## 赋值

Colgm 允许以下赋值运算符：

```rust
a += b; // 加
a -= b; // 减
a *= b; // 乘
a /= b; // 除
a %= b; // 模，仅用于整数类型
a |= b; // 位或，仅用于整数类型
a ^= b; // 位异或，仅用于整数类型
a &= b; // 位与，仅用于整数类型
```

## 控制流

### While 循环

While 循环与 C 语言中的相同。

```rust
while (1 == 1) {
    ...
    continue;
    break;
}
```

### 条件/分支

条件语句与 C 语言中的相同。
语法与 C 语言相同，
除了关键字 `elsif`，它与 `else if` 用法相同。

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

### 匹配

Colgm 支持 `match` 关键字。
Match 语句仅接受枚举类型。每个分支都不会 fall through。
默认分支是 `_`。

```rust
match (variable) {
    EnumType::a => ...;
    EnumType::b => { ... }
    _ => { ... }
}
```

### For 循环

```rust
for (var i = 0; i < 10; i += 1)  {}

// 等同于：
//
// var i = 0;
// while (i < 10) {
//     i += 1;
// }
```

### Foreach/ForIndex 循环

```rust
foreach (var i; container) {}
// 等同于：
//
// for (var tmp_i = container.iter(); !tmp_i.is_end(); tmp_i = tmp_i.next()) {
//     ...
// }
```

```rust
forindex (var i; container) {}
// 等同于：
//
// for (var tmp_i: u64 = 0; tmp_i < container.iter_size(); tmp_i += 1) {
//     ...
// }
```

## 类型转换

关于自动类型转换，请参见[自动类型转换](#自动类型转换)。

与 Rust 有点类似，使用 `=>` 代替 `as`：

```rust
use std::libc::malloc;

func main() -> i32 {
    var res1 = 0 => i32;
    var res2: i32 = 0; // 与上面相同，使用自动类型转换
    var ptr1 = malloc(1024) => i16*;
    return res1 + res2;
}
```

## 结构体

结构体与 C 语言中的相同。
__目前不支持 RAII__。
因此，在将一个结构体移动到另一个结构体时要小心，
这将进行__浅__拷贝。

```rust
struct StructName {
    field1: i64,
    field2: i64
}

// 使用泛型
struct StructWithGeneric<T> {
    __size: i64,
    __data: T*
}
```

### 结构体初始化器

Colgm 支持结构体初始化器，
某些成员字段可以省略。
但要注意不要初始化所有字段。
如果使用了未初始化的字段，
将发生未定义行为。

目前 Colgm 将结构体的默认数据设置为 llvm 的 `zeroinitializer`。

```rust
func test() {
    var s1 = StructName {};
    var s2 = StructName { field1: 1 };
    var s3 = StructName { field1: 1, field2: 2 };
}
```

## 实现

Colgm 支持 `impl` 关键字。
对于方法，第一个参数是 `self`，
它是指向结构体的指针。
对于静态方法，第一个参数不应该是 `self`。

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

// 实现泛型
impl StructWithGeneric<T> {
    ...
}
```

## 枚举

枚举与 C++ 语言中的 `enum class` 相同。
默认情况下，枚举的第一个成员是 `0`，
下一个成员是 `1`，然后是 `2`，依此类推。
但您可以指定每个成员的值。

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

## 编译器内建函数

有一些特殊需求很难用常规的写法满足，比如你想要知道当前程序的编译时间戳，
此时我们需要一个字符串常量来保存编译时间戳，但是很难通过常规写法获取。
于是编译器提供了以下内建函数来获取编译时间戳：

```rust
use std::io::{ io };

fn main() -> i32 {
    io::stdout().out(__time__()).endln();
    return 0;
}
```

现在 colgm 支持以下这些内建函数：

- `__time__`

## 条件编译

条件编译是一种特性，
允许您根据特定条件编译代码。
例如，跨平台问题可以通过条件编译解决。

```rust
#[enable_if(target_os = "linux")]
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

目前支持的条件编译选项：

- `#[enable_if(target_os = "xx", arch = "xx)]`
  - `target_os`: `linux", "windows", "macos"
  - `arch`: `x86_64", "aarch64"
- `#[is_trivial(T)]`
- `#[is_non_trivial(T)]`
- `#[is_pointer(T)]`
- `#[is_non_pointer(T)]`
