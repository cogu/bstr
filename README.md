![unit tests](https://github.com/cogu/bstr/workflows/unit%20tests/badge.svg)

# bstr
Bounded string library for the C programming language.

## What is it?

Some programming languages have a data type called a *view*, which allows data to be referenced without copying it. The bstr library provides a portable *string view* for C-based projects.

This library defines a bounded string, or bstr, as any sequence of bytes between two pointers (`begin` and `end`).

Bounded strings do not have a null character at the end, unlike C strings.
The `begin` pointer points to the first byte in the string, while `end` points to the first byte after the string.
The length of the string can be calculated using pointer arithmetic:

```c
uint32_t len = (uint32_t) (end - begin);
```

## Where is it used?

* [cogu/dtl_json](https://github.com/cogu/dtl_json)
* [cogu/c-apx](https://github.com/cogu/c-apx)

This repository is used as a submodule by the top-level [cogu/c-apx](https://github.com/cogu/c-apx) project.

## Dependencies

* [cogu/adt](https://github.com/cogu/adt)
* [cogu/cutil](https://github.com/cogu/cutil)

The unit test build assumes that the repositories are cloned separately as siblings in a common directory:

* adt
* bstr (this repo)
* cutil

**Git Example**

```bash
cd ~
mkdir repo && cd repo
git clone https://github.com/cogu/adt.git
git clone https://github.com/cogu/bstr.git
git clone https://github.com/cogu/cutil.git
cd bstr
```


## Building with CMake

First, clone this repository and its dependencies into a common directory (such as `~/repo`), as shown above. Alternatively, the repositories can be submodules of a top-level project, as in [cogu/c-apx](https://github.com/cogu/c-apx).

On Windows, use a Native Tools Command Prompt from your Visual Studio installation. It includes a CMake binary that selects the appropriate compiler version by default.


### Using CMake Presets (Clang 18 + Ninja)

```bash
# Run unit tests
cmake --preset clang-test
cmake --build --preset clang-test
ctest --preset clang-test

# Address and Undefined Behavior Sanitizers (ASan + UBSan)
cmake --preset clang-asan
cmake --build --preset clang-asan
ctest --preset clang-asan

# Static Analysis
cmake --preset clang-tidy
cmake --build --preset clang-tidy
```

### Manual CMake Workflows (Linux and Windows)

For Windows, use a "Native tools command prompt" from your Visual Studio installation. It comes with a cmake binary that by default chooses the appropriate compiler version.

#### Running unit tests

Configure:

```sh
cmake -S . -B build-test -GNinja -DUNIT_TEST=ON
```

Build:

```sh
cmake --build build-test
```

Run test cases:

```sh
ctest --test-dir build-test --output-on-failure
```

### CMake Options

| CMake Option | Usage | Default | Description |
|---|---|---|---|
| `UNIT_TEST` | `-DUNIT_TEST=ON` | `OFF` | Enables building the unit test executable (`bstr_unit`) |
| `LEAK_CHECK` | `-DLEAK_CHECK=ON` | `OFF` | Enables memory leak detection via CMemLeak |
| `BSTR_SANITIZERS` | `-DBSTR_SANITIZERS=address,undefined` | `""` | Enables compiler sanitizers (GCC or Clang) |
| `ENABLE_MSVC_ANALYZE` | `-DENABLE_MSVC_ANALYZE=ON` | `OFF` | Enables MSVC static code analysis (`/analyze`) |