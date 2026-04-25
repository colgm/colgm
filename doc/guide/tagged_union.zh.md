# <img src="../logo/colgm.svg" height="50px"/> 标记联合 (Tagged Union)

## 定义

有两种方式可以定义 union：

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

## 初始化

你可以使用以下两种方式初始化 union：

```rust
var a = mir::m_block {a: 1, b: 2, c: 3};
var b = mir::m_num(1024);
var c = mir::m_str(str::from("hello world"));
```

这两种方式等价于旧版本：

```rust
var a = mir { m_block: mir_block {a: 1, b: 2, c: 3} };
var b = mir { m_num: 1024 };
var c = mir { m_str: str::from("hello world") };
```

实际上，新版本是旧版本的语法糖。

## 实现

实现很简单，与实现结构体类似。在获取元素时，
只需使用 `match` 语句来匹配标签。
此外，静态方法也是允许的。

```rust
impl mir {
    pub func generate_mir_from_m_block() -> mir {
        // 创建标记联合实例, 每次只能初始化一个标签
        return mir::m_block(mir_block::instance());
    }

    pub func fetch_m_block_from_mir(self) -> mir_block {
        // match 语句, 接受 union 指针（depth=1）或实例
        match (self) {
            mir_kind::m_block => return self.m_block;
            //                               ^^^^^^^ 获取元素
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
