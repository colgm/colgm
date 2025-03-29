# Feature: Tagged Union

## Syntax

```rust
pub enum mir_kind {
    m_block,
    m_num,
    m_str
}

pub union(mir_kind) mir {
    m_block: mir_block ,
    m_num: mir_num,
    m_str: mir_str
}

pub func generate_mir_from_m_block() -> mir {
    return mir { m_block: mir_block::instance() };
}

pub func fetch_m_block_from_mir(m: mir) -> mir_block {
    match (m) {
        mir_kind::m_block => return m.m_block;
        _ => { unreachable(); }
    }

    // unreachable
    return mir_block::instance();
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
