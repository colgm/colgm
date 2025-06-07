# Feature: defer

Work in progress.

## Syntax

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

Equivalent to:

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
