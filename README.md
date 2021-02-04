![unit tests](https://github.com/cogu/bstr/workflows/unit%20tests/badge.svg)

# bstr
Bounded string library for the C programming language.

## What is it?

Some programming languages has a data type called *view* which allows
variables to be created without the need of copying data. The bstr library offers a portable *string view* which can be used in C-based projects.

This library defines a bounded string (or bstr) to be any sequence of bytes between two pointers (pBegin and pEnd).

Bounded strings does not have a null-character at the end as opposed to C strings (or cstr).
Instead the pointer pBegin points to the first byte in the string while pEnd points to the first byte after the string.
The length of the string can be calculated by using pointer arithmetics:

uint32_t len = (uint32_t) (pEnd - pBegin);

## Where is it used?

* [cogu/dtl_json](https://github.com/cogu/dtl_json)
* [cogu/c-apx](https://github.com/cogu/c-apx)

This repo is a submodule of the [cogu/c-apx](https://github.com/cogu/c-apx) (top-level) project.

## Dependencies

* [cogu/adt](https://github.com/cogu/adt)
* [cogu/cutil](https://github.com/cogu/cutil)

The unit test project assumes that repos are cloned (separately) into a common directory as seen below.

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

First clone this repo and its dependencies into a common directory (such as ~/repo) as seen above. Alternatively the repos can be submodules of a top-level repo (as seen in [cogu/c-apx](https://github.com/cogu/c-apx)).

For Windows, use a "Native tools command prompt" from your Visual Studio installation. It comes with a cmake binary that
by default chooses the appropriate compiler version.

### Running unit tests (Linux and Windows)

Configure:

```sh
cmake -S . -B build -DUNIT_TEST=ON
```

Build:

```sh
cmake --build build --target bstr_unit
```

Run test cases:

```cmd
cd build && ctest
```
