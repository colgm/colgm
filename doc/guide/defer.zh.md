# <img src="../logo/colgm.svg" height="50px"/> Defer

Defer 是一个关键字，允许你在代码块结束前执行一条语句：

- `continue` 和 `break`
- `return` 语句
- 代码块结束

## 语法与语义

Defer 支持单条语句和块语句。

```rust
func main() -> i32 {
    var a = str::from("hello world!");
    defer a.delete();

    var b = str::from("foo bar");
    defer { b.delete(); }

    var c = str::from("baz qux");
    var d = str::from("quux corge");
    defer {
        c.delete();
        d.delete();
    }

    for (var i = 0; i < 10; i += 1) {
        var tmp = str::from_i64(i);
        defer tmp.delete();

        if (i == 5) {
            break;
        } else {
            continue;
        }
    }

    return 0;
}
```

此语言中的 defer 按顺序执行，而非逆序（FIFO）。
等价于：

```rust
func main() -> i32 {
    var a = str::from("hello world!");
    var b = str::from("foo bar");
    var c = str::from("baz qux");
    var d = str::from("quux corge");

    for (var i = 0; i < 10; i += 1) {
        var tmp = str::from_i64(i);

        if (i == 5) {
            tmp.delete();
            break;
        } else {
            tmp.delete();
            continue;
        }
        tmp.delete();
    }

    a.delete();
    b.delete();
    c.delete();
    d.delete();
    return 0;
}
```

## RAII 用法

Defer 可用于在此语言中实现 RAII。

```rust
func foo() -> str {
    var s = str::from("hello world!");
    defer s.delete();

    return s.clone();
}
```

其实际行为如下：

```rust
func foo() -> str {
    var s = str::from("hello world!");
    var defer.tmp.0 = s.clone();
    //  ^^^^^^^^^^^ 由 defer 替换器生成的名称
    //              用户无法编写此类代码

    s.delete();
    return defer.tmp.0;
}
```
