# Tagged Union

## Definition

There're two ways to define tagged union:

```rust
pub enum mir_tag {
    m_block,
    m_num,
    m_str
};

pub union(mir_tag) mir {
    m_block: mir_block,
    m_num: i64,
    m_str: str
};

pub union(enum) mir_without_tag {
    m_block: mir_block,
    m_num: i64,
    m_str: str
}
```

## Initialization

You could use these two ways to initialize tagged union:

```rust
var a = mir::m_block {a: 1, b: 2, c: 3};
var b = mir::m_num(1024);
var c = mir::m_str(str::from("hello world"));
```

These two ways are equivalent to the old version:

```rust
var a = mir { m_block: mir_block {a: 1, b: 2, c: 3} };
var b = mir { m_num: 1024 };
var c = mir { m_str: str::from("hello world") };
```

In actual, new version is the syntax sugar of the old version.

## Implementation

The implementation is simple, just like impl structs.
When getting element, just use `match` statement to match the tag.
Also, static methods are allowed.

```rust
impl mir {
    pub func generate_mir_from_m_block() -> mir {
        // create tagged union instance
        // only one tag could be initialized
        return mir::m_block(mir_block::instance());
    }

    pub func fetch_m_block_from_mir(self) -> mir_block {
        // match statement
        // accept union pointer (depth=1) or instance
        match (self) {
            mir_kind::m_block => return self.m_block;
            //                               ^^^^^^^ get element
            _ => { unreachable(); }
        }
        return mir_block::null();
    }

    pub func fetch_m_block_ptr_from_mir(self) -> mir_block* {
        match (self) {
            mir_kind::m_block => return self.m_block.__ptr__();
            _ => { unreachable(); }
        }
        return nil;
    }

    pub func test_match(self) {
        var out = io::stdout();
        match (self) {
            mir_kind::m_block => self.m_block.dump(out);
            mir_kind::m_num => self.m_num.dump(out);
            mir_kind::m_str => self.m_str.dump(out);
        }
    }
}

impl mir_without_block {
    pub func test_match(self) {
        var out = io::stdout();
        match (self) {
            m_block => self.m_block.dump(out);
            m_num => self.m_num.dump(out);
            m_str => self.m_str.dump(out);
        }
    }
}
```
