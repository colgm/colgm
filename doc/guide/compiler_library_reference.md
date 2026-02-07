# Compiler Library Reference

![just a picture](../jpg/llvm-maybe.jpg)

- [Util](#util)
  - [util::clang_finder::find_clang()](#utilclang_finderfind_clang)
  - [util::timestamp](#utiltimestamp)

## Util

### `util::clang_finder::find_clang()`

Find clang executable path.

```rust
use std::io::{ io };
use util::clang_finder::{ find_clang };

pub func main() -> i32 {
    var clang_path = find_clang();
    defer clang_path.delete();

    io::stdout().out(clang_path.c_str).endln();
    return 0;
}
```

### `util::timestamp`

Include `timestamp` struct and `maketimestamp()` function.

```rust
pub func maketimestamp() -> timestamp;
```

Example:

```rust
pub func main() -> i32 {
    var ts = maketimestamp();
    ts.stamp();

    // do something
    // ...

    var duration_sec = ts.elapsed_sec();   // f64
    var duration_msec = ts.elapsed_msec(); // f64
    var duration_usec = ts.elapsed_usec(); // f64
    var duration_nsec = ts.elapsed_nsec(); // f64
    return 0;
}
```
