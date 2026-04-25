# <img src="../logo/colgm.svg" height="50px"/> Function Pointer

Special type that describes the entry of a function.
It Shares the same size as normal pointer.

## Basic Syntax

```rust
func add(a: i32, b: i32) -> i32 {
    return a + b;
}

func main() -> i32 {
    var f = add;
    var f_type_desc: func(i32, i32) -> i32 = add;

    return f(1, 2) - f_type_desc(0, 3);
}
```
