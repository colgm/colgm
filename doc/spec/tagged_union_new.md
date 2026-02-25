# Tagged Union Initialization (New Syntax)

## Motivation

The current tagged union initialization syntax is verbose:

```rust
var a = mir { m_block: mir_block {a: 1, b: 2, c: 3} };
var b = mir { m_num: mir_num {} };
var c = mir { m_str: mir_str {} };
```

This requires explicitly writing the union name, followed by a brace-enclosed initializer with the tag name and the struct initialization. This spec proposes a new, more concise syntax.

## New Syntax

### Basic Form

The new syntax uses the `::` operator to specify the tag:

```rust
Union::Tag(expr)
```

Where:
- `Union` is the tagged union type name
- `Tag` is the tag name (variant name)
- `expr` is the expression to initialize the data associated with the tag

### Examples

```rust
// Old syntax
var a = mir { m_block: mir_block {a: 1, b: 2, c: 3} };

// New syntax
var a = mir::m_block {a: 1, b: 2, c: 3};
```

```rust
// Old syntax
var b = mir { m_num: mir_num {} };

// New syntax
var b = mir::m_num {};
```

```rust
// Old syntax
var c = mir { m_str: mir_str {} };

// New syntax
var c = mir::m_str {};
```

### Syntax Sugar for Struct Initialization

When the expression is a struct initialization, the following sugar is available:

```rust
// If expr is a struct initialization: Union::Tag { field: value, ... }
// This is equivalent to: Union::Tag(Struct { field: value, ... })

// With parentheses (explicit)
var a = mir::m_block(mir_block {a: 1, b: 2, c: 3});

// With sugar (recommended)
var a = mir::m_block {a: 1, b: 2, c: 3};
```

### Complete Example

```rust
use std::io::{ io };
use std::libc::{ free };
use std::panic::{ panic, assert };

enum Tag {
    aaa,
    bbb,
    ccc
}

union(Tag) MyUnion {
    aaa: i32,
    bbb: i8*,
    ccc: f32
}

func example() {
    // Old syntax (still supported)
    var old1 = MyUnion { aaa: 42 };
    var old2 = MyUnion { bbb: nil };
    
    // New syntax (recommended)
    var new1 = MyUnion::aaa(42);
    var new2 = MyUnion::bbb(nil);
    
    // With struct sugar
    var new3 = MyUnion::ccc { value: 3.14 };  // if ccc is a struct
    
    // Empty struct sugar
    var new4 = MyUnion::aaa {};  // if aaa is an empty struct
}
```

## Grammar

```ebnf
tagged_union_init ::= type_name '::' tag_name ( expression | '{' field_init_list? '}' )
                    | '{' tag_name ':' expression '}'  ; // old syntax, still supported

expression        ::= ... | tagged_union_init | ...
```

## Semantics

1. `Union::Tag(expr)` creates a new instance of the tagged union `Union` with:
   - The tag set to `Tag`
   - The data initialized with the value of `expr`

2. When `expr` is a struct initialization literal (`{ field: value, ... }`), the parentheses can be omitted:
   - `Union::Tag { field: value, ... }` is equivalent to `Union::Tag(Struct { field: value, ... })`
   - The struct type is inferred from the tag's associated type in the union definition

3. The new syntax is purely syntactic sugar and desugars to the old syntax during parsing/AST construction.

## Compatibility

- The old syntax `Union { Tag: expr }` remains valid and supported
- Both syntaxes can be used interchangeably in the same codebase
- The new syntax is recommended for new code due to its conciseness and clarity

## Match Statement

The new syntax does not affect the `match` statement, which continues to use the existing syntax:

```rust
match (union_instance) {
    Tag::aaa => { /* handle aaa */ }
    Tag::bbb => { /* handle bbb */ }
    _ => { /* default */ }
}
```

## Migration Guide

| Old Syntax                              | New Syntax                    |
|-----------------------------------------|-------------------------------|
| `mir { m_block: mir_block {...} }`      | `mir::m_block {...}`          |
| `mir { m_num: mir_num {} }`             | `mir::m_num {}`               |
| `mir { m_str: mir_str {} }`             | `mir::m_str {}`               |
| `MyUnion { aaa: 42 }`                   | `MyUnion::aaa(42)`            |
| `MyUnion { bbb: some_ptr }`             | `MyUnion::bbb(some_ptr)`      |
