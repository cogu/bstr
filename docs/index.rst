Bounded String Library (bstr)
=============================

**bstr** provides a portable *string view* abstraction for C-based projects (C99 and later).

This library defines a bounded string, or *bstr*, as any sequence of bytes between two pointers (``begin`` and ``end``).
Unlike traditional null-terminated C strings, bounded strings do not require a terminating null byte. The ``begin`` pointer points to the first byte in the string, while ``end`` points to the first byte after the string.
The length of the string can be calculated using pointer arithmetic:

.. code-block:: c

   uint32_t len = (uint32_t)(end - begin);

.. toctree::
   :maxdepth: 2
   :hidden:
   :caption: API Reference

   bstr
   bstr_json
   bstr_error

API Modules
===========

.. list-table::
   :header-rows: 1
   :widths: 25 20 55

   * - Module
     - Header
     - Description
   * - :doc:`bstr`
     - ``bstr.h``
     - Core pointer-pair string operations, search, numeric parsing, trimming, and predicates (no context required).
   * - :doc:`bstr_json`
     - ``bstr.h``
     - Context management and JSON parsing routines (numbers and string literals).
   * - :doc:`bstr_error`
     - ``bstr.h``
     - Error constants and return types for parsing operations.

Features
========

- **Zero-Copy Slicing**: Represent substrings, slices, and tokens by adjusting pointer boundaries without allocating heap memory or copying characters.
- **Parsing Routines**: Parse signed/unsigned integers (including auto-base and prefix detection), floating-point numbers, and JSON numbers and string literals with explicit bounds checking.
- **Scanning & Matching**: Fast byte searching, delimiter pairing with escape handling, prefix/substring matching, and line feed discovery.
- **Trimming & Predicates**: Left, right, and bidirectional whitespace stripping, custom predicate skipping, and ASCII/JSON character classification predicates.
- **C String Conversion**: Helper utilities to allocate null-terminated copies or padded copies of bounded strings when standard C string interoperability is required.
- **ADT Integration**: Built-in support for decoding JSON string literals directly into :c:type:`adt_str_t` dynamic strings.

Quick Example
=============

.. code-block:: c

   #include <stdio.h>
   #include <stdlib.h>
   #include "bstr.h"

   void example(const uint8_t *input, size_t length)
   {
       const uint8_t *begin = input;
       const uint8_t *end = input + length;

       // Trim leading and trailing whitespace without copying
       const uint8_t *str_begin;
       const uint8_t *str_end;
       bstr_strip(begin, end, &str_begin, &str_end);

       // Convert to standard null-terminated C string if needed
       char *cstr = bstr_make_cstr(str_begin, str_end);
       if (cstr != NULL) {
           printf("Trimmed string: %s\n", cstr);
           free(cstr);
       }
   }
