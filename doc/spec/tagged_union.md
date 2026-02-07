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
    m_num: mir_num,
    m_str: mir_str
};

pub union(enum) mir_without_tag {
    m_block: mir_block,
    m_num: mir_num,
    m_str: mir_str
}
```

## Implementation

The implementation is simple, just use `match` statement to match the tag.
Also, static methods are allowed, just like impl structs.

```rust
impl mir {
    pub func generate_mir_from_m_block() -> mir {
        // create tagged union instance
        // only one tag could be initialized
        return mir { m_block: mir_block::instance() };
    }

    pub func fetch_m_block_from_mir(self) -> mir_block {
        // match statement, accept tagged union pointer (depth=1) or instance
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
