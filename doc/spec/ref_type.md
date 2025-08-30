# Reference Type

## Implementation

```rs
func foo(a: i64&);
```

is equivalent to:

```rs
func foo(a: i64*);
```

So we need an ast-lowering pass to convert `i64&` to `i64*`.

For example:

```rs
func main() -> i32 {
    var a = 1;
    foo(a);
    return 0;
}
```

will be lowered to:

```rs
func main() -> i32 {
    var a = 1;
    foo(a.__ptr__());
    //   ^^^^^^^^^^
    return 0;
}
```

## Error

This will trigger an error:

```rs
func main() -> i32 {
    foo(1);
    //  ^ error: cannot convert i64 literal to i64&
    return 0;
}
```
