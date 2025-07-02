# Feature: Tagged Union

Work in progress.

## Definition

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

```rust
impl mir {
    pub func generate_mir_from_m_block() -> mir {
        // create tagged union instance
        return mir { m_block: mir_block::instance() };
    }

    pub func fetch_m_block_from_mir(self) -> mir_block {
        // match statement
        match (self) {
            mir_kind::m_block => return self->m_block;
            //                                ^^^^^^^ get element
            _ => { unreachable(); }
        }
        return mir_block::null();
    }

    pub func fetch_m_block_ptr_from_mir(self) -> mir_block* {
        match (self) {
            mir_kind::m_block => return self->m_block.__ptr__();
            _ => { unreachable(); }
        }
        return nil;
    }

    pub func test_match(self) {
        var out = io::stdout();
        match (self) {
            mir_kind::m_block => self->m_block.dump(out);
            mir_kind::m_num => self->m_num.dump(out);
            mir_kind::m_str => self->m_str.dump(out);
        }
    }
}

impl mir_without_block {
    pub func test_match(self) {
        var out = io::stdout();
        match (self) {
            m_block => self->m_block.dump(out);
            m_num => self->m_num.dump(out);
            m_str => self->m_str.dump(out);
        }
    }
}
```
