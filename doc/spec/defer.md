# <img src="../logo/colgm.svg" height="50px"/> Defer (experimental)

Defer is a keyword that allows you to run a statement before the code block ends:

- `continue` and `break`
- `return` statement
- code block ends

__CAUTION__: Defer is now an experimental feature. The specified
statement will run before the return statement now, but the real
design expects it run between the expression in the return statement and the real return operation:

```rust
var s = str::from("hello world!");
defer s.delete();

/* other statements */

return s.substr(0, 5);
```

Now the operation is like this:

```rust
s.delete();
return s.clone(); // as you see, UAF here.
```

But the real design expects this:

```rust
var tmp = s.clone();
//        ^^^^^^^^^-----+ return expression runs before defer
s.delete(); //          |
return tmp; //          |
//     ^^^--------------+ return safely
```

## Syntax and Semantics

Defer allows single statement and block statement.

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

Defer in this language runs in order, not reverse (FIFO).
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
