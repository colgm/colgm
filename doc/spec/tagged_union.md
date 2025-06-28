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
```

## Implementation

```rust
pub func generate_mir_from_m_block() -> mir {
    return mir::m_block(mir_block::instance());
}

pub func fetch_m_block_from_mir(m: mir) -> mir_block {
    match (m) {
        mir_kind::m_block(value) => return value;
        _ => { unreachable(); }
    }
}

pub func fetch_m_block_ptr_from_mir(m: mir) -> mir_block* {
    match (m) {
        mir_kind::m_block(value) => return value.__ptr__();
        _ => { unreachable(); }
    }
}

pub func test_match(m: mir) {
    var out = io::stdout();
    match (m) {
        mir_kind::m_block(value) => value.dump(out);
        mir_kind::m_num(value) => value.dump(out);
        mir_kind::m_str(value) => value.dump(out);
    }
}
```
