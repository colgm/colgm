# Reference Type

## Implementation

```rs
func foo(a: i64&);
```

is equivalent to:

```rs
func foo(a: i64*);
```

So in mir2sir pass, we should convert `i64&`(current type) to `i64*`(real llvm type).

For example:

```rs
func main() -> i32 {
    var a = 1;
    foo(a);
    //  ^ at semantic stage
    //    we will mark this argument as a reference (i64&)
    return 0;
}
```

will be converted to (equivalent to):

```rs
func main() -> i32 {
    var a = 1;
    foo(a.__ptr__());
    //   ^^^^^^^^^^ at mir2sir stage
    //   we will not dereference the pointer of the argument
    return 0;
}
```

## Error

This will trigger an error,
because of trying to convert a literal to a reference:

```rs
func main() -> i32 {
    foo(1);
    //  ^ error: cannot convert i64 literal to i64&
    return 0;
}
```
