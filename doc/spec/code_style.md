# Struct Definitions

```rs
pub struct name_of_this_struct {
    name_of_field_1: name_of_type_1,
    name_of_field_2: name_of_type_2
// ^ 4 space indentation
}
```

# Functions

```rs
pub func name_of_the_function(param_1: type_1, param_2: type_2) -> return_type_name {
    ......
}
```

# Impls

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

# Statements

```rs
if (xxx) {
    ...
} else if (xxx) {
    ...
} else {
    ...
}
```

# Characters Per Line
One single line should not contain more than 90 characters (just an advice), if characters exceed,
use the llvm style:

```rs
pub func foo(param_1: type_1,
             param_2: type_2,
             param_2: type_3) -> return_type {
    ...
}
```
