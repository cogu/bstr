# bstr
Bounded string library for the C programming language.

# What is it?
This library defines a bounded string (or bstr) to be any sequence of bytes between two pointers (pBegin and pEnd).

Bounded strings does not have a null-character at the end as opposed to C strings (or cstr). 
Instead the pointer pBegin points to the first byte in the string while pEnd points to the first byte after the string.
The length of the string can be calculated by using pointer arithmetics:

uint32_t len = (uint32_t) (pEnd - pBegin);

Bounded strings (bstr) are used for building high performing lexers and parses as it does not require any modification of the parsed byte stream nor any copying of memory into null-terminated strings (or cstr).

# Dependencies

* [cogu/adt](https://github.com/cogu/adt)
* [cogu/cutil](https://github.com/cogu/cutil)

This repo depends on some of my other repos. The unit test project(s) assumes that the repos are cloned (separately) into the same directory using the names below.

* adt
* bstr (this repo)
* cutil



