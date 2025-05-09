# Compiler Library Reference

![just a picture](../jpg/llvm-maybe.jpg)

- [Error Handling](#error-handling)
  - [err::panic::assert](#errpanicassert)
  - [err::panic::panic](#errpanicpanic)
  - [err::panic::unreachable](#errpanicunreachable)
  - [err::panic::unimplemented](#errpanicunimplemented)
- [Standard Containers](#standard-containers)
  - [std::list::list](#stdlistlistt)
  - [std::list::primitive_list](#stdlistprimitive_listt)
  - [std::queue::queue](#stdqueuequeuet)
  - [std::queue::primitive_queue](#stdqueueprimitive_queuet)
  - [std::vec::vec](#stdvecvect)
  - [std::vec::primitive_vec](#stdvecprimitive_vect)
  - [std::set::hashset](#stdsethashsett)
  - [std::map::hashmap](#stdmaphashmapk-v)
- [Standard Trivial Type Wrapper](#standard-trivial-type-wrapper)
  - [std::ptr::ptr](#stdptrptrt)
  - [std::ptr::basic](#stdptrbasict)
- [TCP/UDP Utils](#tcpudp-utils)
  - [std::tcp](#stdtcp)
  - [std::udp](#stdudp)

## Error Handling

### err::panic::assert

If condition is false, the program will panic with the error message.

```rust
assert(condition, "error message");
```

### err::panic::panic

Panic with the error message.

```rust
panic("error message");
```

### err::panic::unreachable

Panic with the error message "unreachable".

```rust
unreachable();
```

### err::panic::unimplemented

Panic with the error message "unimplemented".

```rust
unimplemented();
```

## Standard Containers

### `std::list::list<T>`

Source: [`list<T>`](../../src/std/list.colgm)

Linked list, `T` accepts types with these methods:

- `func copy_instance(self) -> T`
- `func delete(self)`

Example:

```rs
func main() -> i32 {
    var a = list<str>::instance();
    var s = str::from("hello world!");
    a.push_back(s.__ptr__()); // s will do copy here

    a.delete();
    s.delete();
    return 0;
}
```

### `std::list::primitive_list<T>`

Source: [`primitive_list<T>`](../../src/std/list.colgm)

Linked list, `T` accepts primitive types and trivially copyable types.

Example:

```rs
func main() -> i32 {
    var a = primitive_list<i32>::instance();
    a.push_back(1);
    a.delete();
    return 0;
}
```

### `std::queue::queue<T>`

Source: [`queue<T>`](../../src/std/queue.colgm)

Queue, `T` accepts types with these methods:

- `func copy_instance(self) -> T`
- `func delete(self)`

Example:

```rs
func main() -> i32 {
    var a = queue<str>::instance();
    var s = str::from("hello world!");
    a.push(s.__ptr__());
    a.pop();

    a.delete();
    s.delete();
    return 0;
}
```

### `std::queue::primitive_queue<T>`

Source: [`primitive_queue<T>`](../../src/std/queue.colgm)

Queue, `T` accepts primitive types and trivially copyable types.

Example:

```rs
func main() -> i32 {
    var a = primitive_queue<i32>::instance();
    a.push(1);
    a.pop();

    a.delete();
    return 0;
}
```

### `std::vec::vec<T>`

Source: [`vec<T>`](../../src/std/vec.colgm)

Vector, `T` accepts types with these methods:

- `func copy(self) -> T*`
- `func copy_instance(self) -> T`
- `func delete(self)`

Example:

```rs
func main() -> i32 {
    var a = vec<str>::instance();
    var s = str::from("hello world!");
    a.push_back(s.__ptr__());
    a.pop_back();

    a.delete();
    s.delete();
    return 0;
}
```

### `std::vec::primitive_vec<T>`

Source: [`primitive_vec<T>`](../../src/std/vec.colgm)

Vector, `T` accepts primitive types and trivially copyable types.

Example:

```rs
func main() -> i32 {
    var a = primitive_vec<i32>::instance();
    a.push_back(1);
    a.pop_back();

    a.delete();
    return 0;
}
```

### `std::set::hashset<T>`

Source: [`hashset<T>`](../../src/std/set.colgm)

Hashset, `T` accepts types with these methods:

- `func copy(self) -> T*`
- `func copy_instance(self) -> T`
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

### `std::map::hashmap<K, V>`

Source: [`hashmap<K, V>`](../../src/std/map.colgm)

Hashmap, `K` and `V` accepts types with these methods:

- `func copy(self) -> K*`
- `func copy_instance(self) -> K`
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

### `std::ptr::ptr<T>`

Source: [`ptr<T>`](../../src/std/ptr.colgm)

Pointer to `T`, trivially copyable.
Pointer needs to be freeed by user manually.
This wrapper could make pointers able to be used
in `list`, `vec`, `hashset`, `hashmap`, etc.

Example:

```rs
func main() -> i32 {
    var p = malloc(i32::__size__()) => i32*;
    var a = vec<ptr<i32>>::instance();
    a.push_back(ptr<i32>::wrap(p).__ptr__());
    
    free(p => i8*); // free manually
    a.delete();
    return 0;
}
```

### `std::ptr::basic<T>`

Source: [`basic<T>`](../../src/std/ptr.colgm)

Basic type, `T` accepts primitive types and trivially copyable types.
This wrapper could make basic types able to be used
in `list`, `vec`, `hashset`, `hashmap`, etc.

Example:

```rs
func main() -> i32 {
    var a = vec<basic<i32>>::instance();
    a.push_back(basic<i32>::wrap(1).__ptr__());
    a.delete();
    return 0;
}
```

## TCP/UDP Utils

### std::tcp

See example for `std::tcp` in [test/socket/tcp_example.colgm](../../test/socket/tcp_example.colgm)

### std::udp

See example for `std::udp` in [test/socket/udp_example.colgm](../../test/socket/udp_example.colgm)
