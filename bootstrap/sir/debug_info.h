#pragma once

#include "colgm.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <sstream>

namespace colgm {

enum class DI_kind {
    DI_error,
    DI_named_metadata,
    DI_ref_index,
    DI_list,
    DI_i32,
    DI_string,
    DI_file,
    DI_compile_unit,
    DI_basic_type,
    DI_structure_type,
    DI_enum_type,
    DI_enumerator,
    DI_subprogram
};

class DI_node {
public:
    static const u64 DI_ERROR_INDEX = SIZE_MAX;
protected:
    DI_kind kind;
    u64 index;

protected:
    void dump_index(std::ostream& out) const {
        if (index != DI_node::DI_ERROR_INDEX) {
            out << "!" << index << " = ";
        }
    }

public:
    DI_node(DI_kind k, u64 i): kind(k), index(i) {}
    ~DI_node() = default;
    virtual void dump(std::ostream&) const;
    bool is(DI_kind k) {
        return kind == k;
    }
};

class DI_named_metadata: public DI_node {
private:
    std::string name;
    std::vector<DI_node*> nodes;

public:
    DI_named_metadata(const std::string& n):
        DI_node(DI_kind::DI_named_metadata, DI_node::DI_ERROR_INDEX),
        name(n) {}
    ~DI_named_metadata() {
        for (auto n: nodes) {
            delete n;
        }
    }
    void add(DI_node* n) { nodes.push_back(n); }
    void dump(std::ostream&) const override;
};

class DI_ref_index: public DI_node {
private:
    u64 ref_index;

public:
    DI_ref_index(u64 fi):
        DI_node(DI_kind::DI_ref_index, DI_node::DI_ERROR_INDEX),
        ref_index(fi) {}
    ~DI_ref_index() = default;
    void dump(std::ostream&) const override;
};

class DI_list: public DI_node {
private:
    std::vector<DI_node*> nodes;

public:
    DI_list(u64 i): DI_node(DI_kind::DI_list, i) {}
    ~DI_list() {
        for (auto n: nodes) {
            delete n;
        }
    }
    void add(DI_node* n) { nodes.push_back(n); }
    void dump(std::ostream&) const override;
};

class DI_i32: public DI_node {
private:
    i32 value;

public:
    DI_i32(i32 v):
        DI_node(DI_kind::DI_i32, DI_node::DI_ERROR_INDEX), value(v) {}
    ~DI_i32() = default;
    void dump(std::ostream&) const override;
};

class DI_string: public DI_node {
private:
    std::string value;

public:
    DI_string(const std::string& v):
        DI_node(DI_kind::DI_string, DI_node::DI_ERROR_INDEX),
        value(v) {}
    ~DI_string() = default;
    void dump(std::ostream&) const override;
};

class DI_file: public DI_node {
private:
    std::string filename;
    std::string directory;

public:
    DI_file(u64 i, const std::string& f, const std::string& d):
        DI_node(DI_kind::DI_file, i),
        filename(f), directory(d) {}
    ~DI_file() = default;
    void dump(std::ostream&) const override;
};

class DI_compile_unit: public DI_node {
private:
    std::string producer;
    u64 file_index;
    u64 imports_index;

public:
    DI_compile_unit(u64 i, const std::string& p, u64 fi):
        DI_node(DI_kind::DI_compile_unit, i),
        producer(p), file_index(fi), imports_index(DI_node::DI_ERROR_INDEX) {}
    ~DI_compile_unit() = default;
    void dump(std::ostream&) const override;
    void set_imports_index(u64 i) { imports_index = i; }
};

class DI_basic_type: public DI_node {
private:
    std::string name;
    u64 size_in_bits;
    std::string encoding;

public:
    DI_basic_type(u64 i, const std::string& n, u64 s, const std::string& e):
        DI_node(DI_kind::DI_basic_type, i),
        name(n), size_in_bits(s), encoding(e) {}
    ~DI_basic_type() = default;
   void dump(std::ostream&) const override;
};

class DI_structure_type: public DI_node {
private:
    std::string name;
    std::string identifier;
    u64 file_index;
    u64 line;

public:
    DI_structure_type(u64 i, const std::string& n,
                      const std::string& id,
                      u64 fi, u64 l):
        DI_node(DI_kind::DI_structure_type, i),
        name(n), identifier(id), file_index(fi), line(l) {}
    ~DI_structure_type() = default;
    void dump(std::ostream&) const override;
};

class DI_enum_type: public DI_node {
private:
    std::string name;
    std::string identifier;
    u64 file_index;
    u64 line;
    u64 base_type_index;
    u64 elements_index;

public:
    DI_enum_type(u64 i, const std::string& n,
                 const std::string& id,
                 u64 fi, u64 l, u64 bti):
        DI_node(DI_kind::DI_enum_type, i),
        name(n), identifier(id), file_index(fi), line(l),
        base_type_index(bti), elements_index(DI_node::DI_ERROR_INDEX) {}
    ~DI_enum_type() = default;
    void dump(std::ostream&) const override;
    void set_elements_index(u64 ei) { elements_index = ei; }
};

class DI_enumerator: public DI_node {
private:
    std::string name;
    i32 value;

public:
    DI_enumerator(u64 i, const std::string& n, i32 v):
        DI_node(DI_kind::DI_enumerator, i), name(n), value(v) {}
    ~DI_enumerator() = default;
    void dump(std::ostream&) const override;
};

class DI_subprogram: public DI_node {
private:
    std::string name;
    u64 file_index;
    u64 line;

public:
    DI_subprogram(u64 i, const std::string& n, u64 fi, u64 l):
        DI_node(DI_kind::DI_subprogram, i),
        name(n), file_index(fi), line(l) {}
    ~DI_subprogram() = default;
    void dump(std::ostream&) const override;
};

}