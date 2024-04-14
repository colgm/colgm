# Colgm Compiler Project: Bootstrap

<img src="../doc/png/logo-small.png" style="width:200px"></img>

[![bootstrap](https://github.com/colgm/colgm/actions/workflows/ci.yml/badge.svg)](https://github.com/colgm/colgm/actions/workflows/ci.yml)

This directory stores the source file of colgm compiler which is used to bootstrap.

## Build

In the top level directory, use these commands:

```sh
mkdir build
cd build
cmake ../bootstrap
make -j
```

## Usage

Use this command to get usage:

```sh
./colgm -h
```

## Syntax

### Bitwise Operator

Colgm now supports these bitwise operators:

```typescript
a | b;
a ^ b;
a & b;
a |= b;
a ^= b;
a &= b;
```

### Logical Operator

Support basic logical operators: `==` `!=` `<` `<=` `>` `>=`.

Logical operators for `and`/`or` all have two types:

```typescript
1==1 and 2==2 && 3==3;
1==1 or 2==2 || 3==3;
```

### Definition

Colgm allows two kinds of definition.

```typescript
var variable_name: type = expression; # with type
var variable_name = expression; # without type
```
