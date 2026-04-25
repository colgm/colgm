# <img src="../logo/colgm.svg" height="50px"/> Lambda

## Basic Syntax

Single Line Lambda:

```rust
func main() -> i32 {
    var double = |x: i32| x * 2;
    var square = |x: i32| x * x;

    return double(2) - square(2);
}
```

Multi Line Lambda:

```rust
func main() -> i32 {
    var double = |x: i32| {
        var y = x * 2;
        return y;
    }

    var square = |x: i32| {
        var y = x * x;
        return y;
    }

    return double(2) - square(2);
}
```

## Parent Scope Capture

Also, lambda could capture parent scope variables.

```rust
func main() -> i32 {
    var a = 1;
    var double = |x: i32| x * a * 2;

    return double(2);
}
```

## Lambda Type

Lambda type is `func(args) -> ret_type`.

```rust
func main() -> i32 {
    var double = |x: i32| x * 2;
    var square = |x: i32| x * x;

    var df: func(i32) -> i32 = double;
    var sf: func(i32) -> i32 = square;

    return df(2) - sf(2);
}
```
