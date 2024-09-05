# <img src="../doc/colgm.svg" height="50px"/> Colgm Compiler

Work in progress.

## Build

First you need bootstrap compiler, then use this command:

```bash
[path of the compiler] main.colgm -L .
```

And `out.ll` is generated, try `lli` or `clang`.

## Run

for `lli`(maybe crash on macOS, don't know why):

```bash
lli out.ll main.colgm -l -a
```

for `clang`:

```bash
clang out.ll -o colgm
./colgm main.colgm -l -a
```
