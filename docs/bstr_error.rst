Error Codes (bstr_error)
========================

Header: ``bstr.h``

The ``bstr`` library defines standard error constants and types for reporting error status during parsing operations (such as JSON parsing).

Data Types
----------

.. doxygentypedef:: bstr_error_t

Error Constants
---------------

.. list-table::
   :header-rows: 1
   :widths: 40 15 45

   * - Constant
     - Value
     - Description
   * - ``BSTR_NO_ERROR``
     - 0
     - Operation succeeded without error.
   * - ``BSTR_PARSE_ERROR``
     - 1
     - Syntax or format error encountered during parsing.
   * - ``BSTR_NUMBER_TOO_LARGE_ERROR``
     - 2
     - Parsed numeric value exceeds the representable range.
   * - ``BSTR_PREMATURE_END_OF_BUFFER_ERROR``
     - 3
     - Input buffer ended unexpectedly before parsing could complete.
   * - ``BSTR_INVALID_CHARACTER_ERROR``
     - 4
     - Encountered an invalid or unexpected character.
   * - ``BSTR_MEM_ERROR``
     - 5
     - Memory allocation failure.

API Reference
-------------

.. doxygendefine:: BSTR_NO_ERROR

.. doxygendefine:: BSTR_PARSE_ERROR

.. doxygendefine:: BSTR_NUMBER_TOO_LARGE_ERROR

.. doxygendefine:: BSTR_PREMATURE_END_OF_BUFFER_ERROR

.. doxygendefine:: BSTR_INVALID_CHARACTER_ERROR

.. doxygendefine:: BSTR_MEM_ERROR
