Bounded String (bstr)
======================

Header: ``bstr.h``

The core ``bstr`` module provides high-performance, non-allocating string operations operating on bounded ranges defined by two pointers (``begin`` and ``end``).

None of the functions in this module require a context object. They operate directly on raw memory buffers and never write a terminating null character into the source buffer.

String Conversion
-----------------

These functions allocate a standard null-terminated C string copy from a bounded range. The caller is responsible for releasing the returned string using ``free()``.

.. doxygenfunction:: bstr_make_cstr

.. doxygenfunction:: bstr_make_cstr_with_padding

Search & Matching
-----------------

Functions to search for individual bytes, delimiters, paired enclosing characters, and prefixes within a bounded range.

.. doxygenfunction:: bstr_find_byte

.. doxygenfunction:: bstr_match_pair

.. doxygenfunction:: bstr_match_bstr

.. doxygenfunction:: bstr_match_cstr

.. doxygenfunction:: bstr_find_line_feed

Numeric Parsing
---------------

Functions for parsing standard integer and floating-point numeric values from bounded buffers without requiring null termination.

.. doxygenfunction:: bstr_parse_double

.. doxygenfunction:: bstr_parse_long

.. doxygenfunction:: bstr_parse_long_long

.. doxygenfunction:: bstr_parse_unsigned_long

.. doxygenfunction:: bstr_parse_unsigned_long_long

Trimming & Skipping
-------------------

Whitespace trimming and predicate-based scanning routines.

.. doxygenfunction:: bstr_lstrip

.. doxygenfunction:: bstr_rstrip

.. doxygenfunction:: bstr_strip

.. doxygenfunction:: bstr_skip_forward_while

.. doxygenfunction:: bstr_skip_backward_while

Character Predicates
--------------------

Predicate functions used with :c:func:`bstr_skip_forward_while` and :c:func:`bstr_skip_backward_while` for character classification.

.. doxygenfunction:: bstr_pred_is_horizontal_space

.. doxygenfunction:: bstr_pred_is_whitespace

.. doxygenfunction:: bstr_pred_is_digit

.. doxygenfunction:: bstr_pred_is_hex_digit

.. doxygenfunction:: bstr_pred_is_nonzero_digit

.. doxygenfunction:: bstr_pred_is_control_char
