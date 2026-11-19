# VEX

**VEX** is a statically typed, compiled programming language and
compiler project built around LLVM. This repository contains the
language and compiler specification, including syntax, type-system
rules, module and package systems, runtime behavior, build outputs, and
toolchain commands.

> **Specification note:** This README summarizes the supplied VEX
> language/compiler reference. A feature being described by the
> specification does not, by itself, prove that every part is already
> implemented or available in every released build. Check the current
> source and release notes for implementation status.

## Contents

-   [Highlights](#highlights)
-   [Language overview](#language-overview)
-   [Example](#example)
-   [Compiler workflow](#compiler-workflow)
-   [Build outputs](#build-outputs)
-   [Targets](#targets)
-   [Toolchain requirements](#toolchain-requirements)
-   [Install a release binary](#install-a-release-binary)
-   [Build from source](#build-from-source)
-   [Command reference](#command-reference)
-   [Modules and packages](#modules-and-packages)
-   [Runtime](#runtime)
-   [Project configuration](#project-configuration)
-   [Documentation](#documentation)

## Highlights

The VEX language specification describes:

-   Static typing with bidirectional type inference.
-   Generic functions and types, trait constraints, and compile-time
    specialization.
-   Structs, classes, enums, traits, interfaces, and operator
    overloading.
-   Ownership-oriented pointers and borrowing forms.
-   Nullable types, null-coalescing, and error propagation.
-   Lambdas, closures, generators, and asynchronous functions.
-   Compile-time evaluation and conditional compilation.
-   A module system with source (`.vex`) and compiled-library (`.vxl`)
    modules.
-   An LLVM-based compilation pipeline.
-   Native executable, shared-library, VEX-library, and WebAssembly
    output modes.
-   C interoperability through exported functions and generated C
    headers.
-   A package-manager design named VPM.

## Language overview

VEX source files use the `.vex` extension. Compiled VEX library modules
use `.vxl`.

### Variables

``` vex
var count := 42       // Mutable; type inferred as int
let name := "VEX"     // Immutable after initialization
const LIMIT = 100     // Compile-time constant
```

A variable's type is fixed after inference. Explicit annotations are
also supported:

``` vex
var port: int32 = 8080
```

### Functions

``` vex
fn add(a: int, b: int) -> int {
    return a + b
}

fn square(x: int) -> int = x * x
```

The language reference specifies named arguments, default parameters,
variadic parameters, generic functions, function types, and overloads
based on parameter types.

### Modules and imports

The `::` operator is reserved for module paths:

``` vex
import std::io

fn main() {
    io::println("Hello from VEX")
}
```

A module path identifies a module file; members inside that module are
accessed after the module is resolved. The specification does not use
`::` for static methods or enum variants.

### Types and data structures

The reference describes primitive types, fixed-size arrays, dynamic
lists, maps, tuples, sets, nullable types, raw pointers, owned pointers,
shared reference-counted pointers, and borrow references.

``` vex
struct Point {
    x: float64
    y: float64
}

fn distanceSquared(p: Point) -> float64 {
    return p.x * p.x + p.y * p.y
}
```

It also specifies classes with single inheritance, enums, traits,
interfaces, properties, and operator overloading.

### Generics and compile-time features

Generic parameters are written with angle brackets:

``` vex
fn identity<T>(value: T) -> T {
    return value
}
```

The specification describes trait bounds, `where` constraints,
compile-time functions and constants, conditional compilation, and
generic specialization.

### Error handling and nullable values

The language reference includes nullable types (`T?`), null coalescing
(`??`), safe calls (`?.`), and a postfix error-propagation operator
(`?`). The exact behavior and typing rules are defined in the language
specification.

### Lambdas, generators, and async

Lambda syntax uses vertical bars:

``` vex
let double: fn(int) -> int = |x| -> x * 2
```

The specification also describes synchronous generators (`fn*` with
`yield`), asynchronous functions (`async fn` with `await`), and
asynchronous generators.

## Example

A small VEX program:

``` vex
import std::io

fn main() {
    let language := "VEX"
    io::println(f"Hello from {language}!")
}
```

The standard-library import and `io::println` call follow the module
syntax described in the specification.

## Compiler workflow

The documented build pipeline is:

1.  Read project configuration from `vex.toml`, when present.
2.  Read and parse the entry source file.
3.  Resolve imports recursively and construct the module dependency
    graph.
4.  Compile modules in dependency order.
5.  Perform type checking, borrow checking, compile-time evaluation, and
    generic specialization.
6.  Lower the program to LLVM IR.
7.  Link modules and requested native libraries, apply optimization
    passes, and generate the requested output.

Imports define the source dependency graph; users are not expected to
manually list every imported source file.

## Build outputs

The compiler specification describes these output modes:

  -----------------------------------------------------------------------
  Mode                                Output
  ----------------------------------- -----------------------------------
  Normal executable build             Native executable

  `--lib`                             Compiled VEX library (`.vxl`)

  `--shared`                          Shared library (`.so`, `.dll`, or
                                      `.dylib`, depending on platform)

  `--lib --shared`                    VEX library and shared library

  `--target wasm`                     WebAssembly module (`.wasm`)
  -----------------------------------------------------------------------

A shared-library build is also specified to generate a C header for
exported functions and VEX metadata for importing the library from VEX.

## Targets

The supplied compiler reference lists these targets:

  Target            Platform
  ----------------- -----------------------
  `linux-x64`       Linux x86-64
  `linux-arm64`     Linux ARM64
  `windows-x64`     Windows x64
  `macos-arm64`     macOS ARM64
  `wasm`            WebAssembly
  `macos-x64`       macOS x64
  `wasm-wasi`       WebAssembly with WASI
  `linux-riscv64`   Linux RISC-V 64-bit
  `android-arm64`   Android ARM64

The reference describes cross-compilation between supported targets.
Actual availability depends on the compiler build, target backend,
sysroot, and required platform toolchain.

## Toolchain requirements

### Building VEX from source

The supplied build instructions require:

-   CMake
-   A C and C++ toolchain
-   Clang (`clang++`)
-   LLVM 18 development headers and CMake package files

The example below expects LLVM's CMake package at:

``` text
/usr/lib/llvm-18/lib/cmake/llvm
```

The exact path may differ by operating system or LLVM installation
method.

### Linker note: LLD

The shown CMake configuration does **not** explicitly select LLD.
CMake/Clang will use the configured default linker unless the build is
configured otherwise.

LLD is only required if your build or VEX's generated-program toolchain
explicitly selects it. On Ubuntu, the LLVM 18 linker package can be
installed with:

``` bash
sudo apt update
sudo apt install lld-18
```

To explicitly ask Clang to use LLD for a link:

``` bash
clang++ -fuse-ld=lld main.o -o main
```

## Install a release binary

The following example installs the Linux x86-64 release into the
directory where it is extracted. It does not permanently modify your
shell configuration.

``` bash
tar xf VEX-1.0.0-linux-x86_64.tar.gz
cd VEX-1.0.0-linux-x86_64

export PATH="$PWD/bin:$PATH"
export LD_LIBRARY_PATH="$PWD/lib:$LD_LIBRARY_PATH"

vexc --version
```

-   `PATH` allows the shell to find the VEX executable.
-   `LD_LIBRARY_PATH` tells the Linux dynamic loader where to look for
    shared libraries shipped in the release's `lib` directory.
-   `vexc --version` checks that the compiler executable can be
    launched.

For a persistent setup, add the appropriate installation paths to your
shell profile after choosing a permanent installation directory.

## Build from source

Example Linux build using LLVM 18:

``` bash
tar xf vex-src-1.0.0.tar.gz
cd vex

cmake . -B build \
  -DLLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm \
  -DCMAKE_BUILD_TYPE=Release \
  -DVEX_BUILD_RUNTIME=ON \
  -DVEX_BUILD_SHARED=ON \
  -DCMAKE_CXX_COMPILER=clang++

cmake --build build -j"$(nproc)"
```

The supplied build instructions identify these expected outputs:

``` text
build/vexc
build/libvexcompiler.so
```

-   `build/vexc` --- VEX compiler executable.
-   `build/libvexcompiler.so` --- shared VEX compiler library.

If LLVM is installed in a different location, update `LLVM_DIR` to the
directory containing `LLVMConfig.cmake`.

## Command reference

The language/compiler reference documents the following driver commands.
Use the command name provided by the actual release package; the release
installation example above verifies the compiler as `vexc`.

### Project setup

``` bash
vex init
vex init --lib
```

### Run

``` bash
vex
vex run
vex run main.vex
vex run -- arg1 arg2
```

### Build

``` bash
vex build
vex build main.vex
vex build main.vex -o myapp

vex build --lib mylib.vex -o mylib.vxl
vex build --shared mylib.vex -o libmylib.so
vex build --lib --shared mylib.vex

vex build --target wasm main.vex
```

### Check and format

``` bash
vex check main.vex
vex fmt main.vex
vex fmt .
```

### Tests and benchmarks

``` bash
vex test
vex test main.vex
vex test --filter "testAdd"
vex bench
```

### Generate documentation

``` bash
vex doc
vex doc --out ./docs
```

### Build options and C libraries

``` bash
vex build -D DEBUG=true
vex build -D MY_FEATURE=true

vex build --link ssl
vex build --link-path /usr/local/lib
vex build --include-path /usr/include
vex build --no-std
```

These commands are taken from the supplied reference; confirm that each
option is supported by the particular compiler binary you are using.

## Modules and packages

The module system uses:

-   `.vex` for source modules.
-   `.vxl` for compiled VEX library modules.

The same import syntax is specified for source and compiled modules. The
compiler resolves imports through the project, installed packages, and
standard library locations, following the resolution order defined in
the language reference. Circular imports are specified as compile
errors.

### VPM

VPM is described as a separate package-manager tool that integrates with
the project configuration in `vex.toml`. The reference describes it as
responsible for installing, updating, and removing dependencies. Check
the release distribution for the availability and exact command syntax
of the `vpm` executable.

## Runtime

The supplied design reference describes `vex_rt` as a static runtime
library linked into generated executables rather than a separate runtime
shared library that users must distribute.

It also describes a `--no-rt` mode for programs that do not need the
standard runtime facilities. The reference lists limitations for this
mode, including facilities that require runtime services such as I/O,
filesystem/network access, threading, and async execution.

## Project configuration

A project may use a `vex.toml` file. The reference gives this
illustrative structure:

``` toml
[project]
name = "myapp"
version = "1.0.0"
entry = "main.vex"
authors = ["Your Name"]

[dependencies]
json = { source = "official", version = "v2.1.0" }

[build]
optimization = "O2"
target = "linux-x64"
output = "myapp"

[c_interop]
link = ["ssl", "sqlite3"]
link_path = ["/usr/local/lib"]
include = ["/usr/local/include"]
```

The exact configuration fields accepted by a particular release should
be checked against that release's implementation.

## Documentation

The VEX language/compiler reference is the authoritative source for
detailed language rules, including operator precedence, symbol
disambiguation, lexical syntax, visibility, type inference, ownership
and borrowing, functions, data structures, modules, compile-time
behavior, compiler stages, runtime behavior, and package management.

Use the reference alongside the source code and release-specific
documentation when implementing or debugging compiler behavior.
