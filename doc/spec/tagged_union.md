# Feature: Tagged Union

Work in progress.

## Definition

```rust
pub union(enum) mir {
    m_block: mir_block ,
    m_num: mir_num,
    m_str: mir_str
}
```

Or we can also write this:

```rust
pub enum mir_tag {
    m_block,
    m_num,
    m_str
};

pub union(mir_tag) mir {
    m_block: mir_block ,
    m_num: mir_num,
    m_str: mir_str
};
```

## Implementation

```rust
pub func generate_mir_from_m_block() -> mir {
    return mir::m_block(mir_block::instance());
}

pub func fetch_m_block_from_mir(m: mir) -> mir_block {
    match (m) {
        mir_kind::m_block => return m.m_block;
        _ => { unreachable(); }
    }
}

pub func test_match(m: mir) {
    var out = io::stdout();
    match (m) {
        mir_kind::m_block => m.m_block.dump(out);
        mir_kind::m_num => m.m_num.dump(out);
        mir_kind::m_str => m.m_str.dump(out);
    }
}
```
