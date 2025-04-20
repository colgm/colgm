# Compiler Library Reference

![just a picture](../jpg/llvm-maybe.jpg)

## `list<T>`

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

## `primitive_list<T>`

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

## `queue<T>`

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

## `primitive_queue<T>`

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

## `vec<T>`

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

## `primitive_vec<T>`

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

## `hashset<T>`

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

## `hashmap<K, V>`

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

## `ptr<T>`

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

## `basic<T>`

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
