JSON Parsing & Context (bstr_json)
==================================

Header: ``bstr.h``

The JSON parsing module in ``bstr`` provides functions to parse JSON numbers and RFC 8259 JSON string literals directly from bounded byte ranges.

Unlike the core bounded-string operations, JSON parsing routines require a parser context (:c:type:`bstr_context_t`) to capture error details when parsing fails. For all other string slicing and parsing operations in the library, no context is needed.

Data Types
----------

.. doxygenstruct:: bstr_context_tag
   :members:

.. doxygentypedef:: bstr_context_t

.. doxygenstruct:: bstr_number_tag
   :members:

.. doxygentypedef:: bstr_number_t

Context Management
------------------

A :c:type:`bstr_context_t` can be allocated either on the stack or dynamically on the heap:

* **Stack allocation**: Initialize using :c:func:`bstr_context_create` and clean up with :c:func:`bstr_context_destroy`.
* **Heap allocation**: Allocate using :c:func:`bstr_context_new` and release with :c:func:`bstr_context_delete`.

Lifecycle Functions
~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: bstr_context_create

.. doxygenfunction:: bstr_context_destroy

.. doxygenfunction:: bstr_context_new

.. doxygenfunction:: bstr_context_delete

Error Inspection
~~~~~~~~~~~~~~~~

When a JSON parsing function returns ``NULL``, inspect the context with :c:func:`bstr_context_last_error` to identify the cause of failure (see :doc:`bstr_error`).

.. doxygenfunction:: bstr_context_last_error

.. doxygenfunction:: bstr_context_clear_error

JSON Parsing Functions
----------------------

.. doxygenfunction:: bstr_parse_json_number

.. doxygenfunction:: bstr_parse_json_string_literal

Example: Parsing a JSON String Literal
--------------------------------------

.. code-block:: c

   #include <stdio.h>
   #include <string.h>
   #include "bstr.h"
   #include "adt_str.h"

   void example_parse_json(void)
   {
       const char *json_snippet = "\"Hello, \\u0041gent!\"";
       const uint8_t *begin = (const uint8_t *)json_snippet;
       const uint8_t *end = begin + strlen(json_snippet);

       bstr_context_t ctx;
       bstr_context_create(&ctx);

       adt_str_t out_str;
       adt_str_create(&out_str);

       const uint8_t *next = bstr_parse_json_string_literal(&ctx, begin, end, &out_str);
       if (next != NULL) {
           printf("Decoded JSON string: %s\n", adt_str_cstr(&out_str));
       } else {
           bstr_error_t err = bstr_context_last_error(&ctx);
           printf("JSON parsing failed with error code: %d\n", err);
       }

       adt_str_destroy(&out_str);
       bstr_context_destroy(&ctx);
   }
