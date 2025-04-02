# Feature: tuple

## Syntax

```rust
pub func main() -> i32 {
    var t = (0, 1, 2, 3); // tuple<i64, i64, i64, i64>
    return t[0] + t[3] - t[2] - t[1];
}
```

```rust
pub func generate_tuple() -> tuple<i64, f64, i8*> {
    return (0, 1.0, "foo");
}
```
