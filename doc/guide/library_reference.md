# Compiler Library Reference

![just a picture](../jpg/llvm-maybe.jpg)

- [Error Handling](#error-handling)
  - [std::panic::assert](#stdpanicassert)
  - [std::panic::panic](#stdpanicpanic)
  - [std::panic::unreachable](#stdpanicunreachable)
  - [std::panic::unimplemented](#stdpanicunimplemented)
- [Standard Containers](#standard-containers)
  - [std::list::list](#stdlistlistt)
  - [std::queue::queue](#stdqueuequeuet)
  - [std::vec::vec](#stdvecvect)
  - [std::set::hashset](#stdsethashsett)
  - [std::pair::pair](#stdpairpairk-v)
  - [std::map::hashmap](#stdmaphashmapk-v)
- [Standard Trivial Type Wrapper](#standard-trivial-type-wrapper)
  - [std::basic::basic](#stdbasicbasict)
- [File Formats](#file-formats)
  - [std::json::json](#stdjsonjson)
- [TCP/UDP Utils](#tcpudp-utils)
  - [std::tcp](#stdtcp)
  - [std::udp](#stdudp)

## Error Handling

### std::panic::assert

If condition is false, the program will panic with the error message.

```rust
assert(condition, "error message");
```

### std::panic::panic

Panic with the error message.

```rust
panic("error message");
```

### std::panic::unreachable

Panic with the error message "unreachable".

```rust
unreachable();
```

### std::panic::unimplemented

Panic with the error message "unimplemented".

```rust
unimplemented();
```

## Standard Containers

### `std::list::list<T>`

Source: [`list<T>`](../../src/std/list.colgm)

Linked list that accepts both trivial and non-trivial types.

- If `T` is non-trivial type, it accepts types with these methods:
  - `func clone(self) -> T`
  - `func delete(self)`
- If `T` is trivial type, it could be used directly.

Example:

```rs
func main() -> i32 {
    var a = list<str>::instance();
    var b = list<i64>::instance();

    var s = str::from("hello world!");
    a.push_back(s.__ptr__()); // s will do copy here
    b.push_back(123);         // do not need pointer type

    a.delete();
    b.delete();
    s.delete();
    return 0;
}
```

### `std::queue::queue<T>`

Source: [`queue<T>`](../../src/std/queue.colgm)

Queue that accepts both trivial and non-trivial types.

- If `T` is non-trivial type, it accepts types with these methods:
  - `func clone(self) -> T`
  - `func delete(self)`
- If `T` is trivial type, it could be used directly.

Example:

```rs
func main() -> i32 {
    var a = queue<str>::instance();
    var b = queue<i32>::instance();
    var s = str::from("hello world!");
    a.push(s.__ptr__());
    a.pop();
    b.push(123);
    b.pop();

    a.delete();
    b.delete();
    s.delete();
    return 0;
}
```

### `std::vec::vec<T>`

Source: [`vec<T>`](../../src/std/vec.colgm)

Vector that accepts both trivial and non-trivial types.

- If `T` is non-trivial type, it accepts types with these methods:
  - `func clone(self) -> T`
  - `func delete(self)`
- If `T` is trivial type, it could be used directly.

Example:

```rs
func main() -> i32 {
    var a = vec<str>::instance();
    var s = str::from("hello world!");
    a.push(s.__ptr__());
    a.pop_back();

    var b = vec<i32>::instance();
    b.push(123);

    a.delete();
    s.delete();
    b.delete();
    return 0;
}
```

### `std::set::hashset<T>`

Source: [`hashset<T>`](../../src/std/set.colgm)

Hashset, `T` accepts types with these methods:

- `func clone(self) -> T`
- `func delete(self)`
- `func hash(self) -> u64`

Example:

```rs
func main() -> i32 {
    var a = hashset<str>::instance();
    var s = str::from("hello world!");
    a.insert(s.__ptr__());
    a.delete();
    s.delete();
    return 0;
}
```

### `std::pair::pair<K, V>`

Source: [`pair<K, V>`](../../src/std/pair.colgm)

Pair, `K` and `V` accept both trivial and non-tribial types.

Example:

```rs
func main() -> i32 {
    var a = pair<str, i32>::instance(s.__ptr__(), 123);
    var b = pair<i32, str>::instance(123, s.__ptr__());
    var c = pair<i32, i32>::instance(123, 456);
    var d = pair<str, str>::instance(s.__ptr__(), s.__ptr__());

    s.delete();
    a.delete();
    b.delete();
    c.delete();
    d.delete();
    return 0;
}
```

### `std::map::hashmap<K, V>`

Source: [`hashmap<K, V>`](../../src/std/map.colgm)

Hashmap, `K` and `V` accepts types with these methods:

- `func clone(self) -> K`
- `func delete(self)`

And `K` must implement `func hash(self) -> u64`.

Example:

```rs
func main() -> i32 {
    var a = hashmap<str, basic<i32>>::instance();
    var s = str::from("hello world!");

    a.insert(s.__ptr__(), basic<i32>::wrap(1).__ptr__());

    a.delete();
    s.delete();
    return 0;
}
```

## Standard Trivial Type Wrapper

### `std::basic::basic<T>`

Source: [`basic<T>`](../../src/std/ptr.colgm)

Basic type, `T` accepts primitive types.
This wrapper could make basic types able to be used in `hashset` and `hashmap`.
Because `hashset` and `hashmap` now need the key type with the method
`func hash(self) -> u64`.

Example:

```rs
func main() -> i32 {
    var a = hashset<basic<i32>>::instance();
    a.insert(basic<i32>::wrap(1));
    a.delete();
    return 0;
}
```

## File Formats

### `std::json::json`

Source: [`json`](../../src/std/json.colgm)

Not stable, methods may change.

```rs
use std::json::{ json };
use std::str::{ str };
use std::libc::{ free };
use std::io::{ io };

func main() -> i32 {
    var input = str::from("{ \"a\": 1, \"b\": 2 }");
    defer input.delete();

    var j = json::parse(input.__ptr__());
    defer {
        j->delete();
        free(j => i8*);
    }

    if (!j->is_invalid()) {
        var s = j->to_string();
        defer s.delete();

        io::stdout().out(s.c_str).endln();
    }

    return 0;
}
```

## TCP/UDP Utils

### std::tcp

See example for `std::tcp` in [test/socket/tcp_example.colgm](../../test/socket/tcp_example.colgm)

### std::udp

See example for `std::udp` in [test/socket/udp_example.colgm](../../test/socket/udp_example.colgm)
