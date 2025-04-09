# Code Style

## Struct Definition

Indentation: 4 spaces

```rs
pub struct name_of_this_struct {
    name_of_field_1: name_of_type_1,
    name_of_field_2: name_of_type_2
// ^ 4 space indentation
}
```

## Function

```rs
pub func name_of_the_function(param_1: type_1,
                              param_2: type_2) -> return_type_name {
    ......
}
```

## Impl

```rs
impl name_of_this_struct {
    pub func static_method(param_1: type_2) -> return_type_name {
        ...
    }

    pub func method(self, param_1: type_1) -> return_type_name {
        ...
    }
}
```

## Statement

```rs
if (xxx) {
    ...
} else if (xxx) {
    ...
} else {
    ...
}

while (xxx) {
    ...
}

match (xxx) {
    xx::xxx => {}
    _ => {}
}
```

## Characters Per Line

One single line should not contain more than 100 characters (just advice),
if control the number of characters to 80 each line, that's better.
If characters exceed, use the llvm style:

```rs
pub func foo(param_1: type_1,
             param_2: type_2,
             param_2: type_3) -> return_type {
    ...
}
```
