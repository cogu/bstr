/*****************************************************************************
* \file      bstr.h
* \author    Conny Gustafsson
* \date      2017-08-04
* \brief     Bounded strings library
*
* Copyright (c) 2017-2026 Conny Gustafsson
* SPDX-License-Identifier: MIT
* See LICENSE in project root for full license terms.
******************************************************************************/
#ifndef BSTR_H
#define BSTR_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include "adt_str.h"

//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

typedef struct bstr_number_tag
{
   uint32_t integer;
   int32_t fraction;
   int32_t exponent;
   bool has_integer;
   bool has_fraction;
   bool has_exponent;
   bool is_negative;
} bstr_number_t;

typedef int32_t bstr_error_t;
#define BSTR_NO_ERROR                       ((bstr_error_t) 0)
#define BSTR_PARSE_ERROR                    ((bstr_error_t) 1)
#define BSTR_NUMBER_TOO_LARGE_ERROR         ((bstr_error_t) 2)
#define BSTR_PREMATURE_END_OF_BUFFER_ERROR  ((bstr_error_t) 3)
#define BSTR_INVALID_CHARACTER_ERROR        ((bstr_error_t) 4)
#define BSTR_MEM_ERROR                      ((bstr_error_t) 5)


typedef struct bstr_context_tag
{
   bstr_error_t last_error;
} bstr_context_t;


//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

/**
 * \brief Initializes a bounded-string context.
 *
 * \param self Context to initialize. May be NULL.
 */
void bstr_context_create(bstr_context_t *self);

/**
 * \brief Allocates and initializes a bounded-string context.
 *
 * \return A newly allocated context, or NULL if allocation fails. The caller
 * must release the context with bstr_context_delete().
 */
bstr_context_t *bstr_context_new(void);

/**
 * \brief Deletes a dynamically allocated bounded-string context.
 *
 * \param self Context created by bstr_context_new(). May be NULL.
 */
void bstr_context_delete(bstr_context_t *self);

/**
 * \brief Returns the last error stored in a bounded-string context.
 *
 * \param ctx Context to inspect. Must not be NULL.
 * \return The stored bstr error code.
 */
bstr_error_t bstr_context_last_error(bstr_context_t *ctx);

/**
 * \brief Clears the last error stored in a bounded-string context.
 *
 * \param ctx Context to clear. Must not be NULL.
 */
void bstr_context_clear_error(bstr_context_t *ctx);

/**
 * \brief Copies a bounded string into a newly allocated C string.
 *
 * \param begin First byte of the source range.
 * \param end One past the last byte of the source range.
 * \return A null-terminated copy, or NULL for an invalid or empty range or if
 * allocation fails. The caller must release the returned string with free().
 */
char* bstr_make_cstr(const uint8_t *begin, const uint8_t *end);

/**
 * \brief Copies a bounded string into a padded region of a new buffer.
 *
 * The bytes before and after the copied range are left uninitialized. The last
 * byte of the allocated buffer is set to the null character.
 *
 * \param begin First byte of the source range.
 * \param end One past the last byte of the source range.
 * \param padding_left Number of bytes reserved to the left of the copied string.
 * \param padding_right Number of bytes reserved to the right of the copied string.
 * \return A newly allocated buffer, or NULL for invalid input or if allocation
 * fails. The caller must release the returned buffer with free().
 */
char* bstr_make_cstr_with_padding(const uint8_t *begin, const uint8_t *end, uint16_t padding_left, uint16_t padding_right);

/**
 * \brief Finds the first occurrence of a byte in a bounded range.
 *
 * \param begin First byte of the range.
 * \param end One past the last byte of the range.
 * \param byte Byte value to find.
 * \return A pointer to the first matching byte, end if no match is found, or
 * NULL if begin is after end.
 */
const uint8_t *bstr_find_byte(const uint8_t *begin, const uint8_t *end, uint8_t byte);

/**
 * \brief Finds the matching closing byte for a nested pair.
 *
 * \param begin First byte of the range; it must equal left.
 * \param end One past the last byte of the range.
 * \param left Opening byte.
 * \param right Closing byte.
 * \param escape_char Byte that escapes the following byte, or zero to disable
 * escape handling.
 * \return A pointer to the matching right byte, begin if the pair is not
 * closed before end, or NULL if the range does not start with left.
 */
const uint8_t *bstr_match_pair(const uint8_t *begin, const uint8_t *end, uint8_t left, uint8_t right, uint8_t escape_char);

/**
 * \brief Matches a bounded string at the start of another bounded range.
 *
 * \param begin First byte of the input range.
 * \param end One past the last byte of the input range.
 * \param str_begin First byte of the string to match.
 * \param str_end One past the last byte of the string to match.
 * \return A pointer immediately after the match, NULL on a byte mismatch or
 * invalid range, or begin if the input ends before the string is fully matched.
 */
const uint8_t *bstr_match_bstr(const uint8_t *begin, const uint8_t *end, const uint8_t *str_begin, const uint8_t *str_end);

/**
 * \brief Matches a null-terminated C string at the start of a bounded range.
 *
 * \param begin First byte of the input range.
 * \param end One past the last byte of the input range.
 * \param cstr Null-terminated string to match.
 * \return A pointer immediately after the match, NULL on a mismatch or invalid
 * argument, or begin if the input ends before cstr is fully matched.
 */
const uint8_t *bstr_match_cstr(const uint8_t *begin, const uint8_t *end, const char *cstr);

/**
 * \brief Parses a double from at most the first 32 bytes of a bounded range.
 *
 * \param begin First byte of the input range.
 * \param end One past the last byte of the input range.
 * \param data Receives the parsed value.
 * \return A pointer after the parsed prefix, begin if no conversion is
 * possible, or NULL if conversion does not produce a valid position.
 */
const uint8_t* bstr_parse_double(const uint8_t* begin, const uint8_t* end, double* data);

/**
 * \brief Parses a long integer using C base-prefix rules.
 *
 * \param begin First byte of the input range.
 * \param end One past the last byte of the input range.
 * \param data Receives the parsed value.
 * \return A pointer after the parsed prefix, begin if no conversion is
 * possible, or NULL if conversion does not produce a valid position.
 */
const uint8_t *bstr_parse_long(const uint8_t *begin, const uint8_t *end, long *data);

/**
 * \brief Parses a long long integer using C base-prefix rules.
 *
 * \param begin First byte of the input range.
 * \param end One past the last byte of the input range.
 * \param data Receives the parsed value.
 * \return A pointer after the parsed prefix, begin if no conversion is
 * possible, or NULL if conversion does not produce a valid position.
 */
const uint8_t* bstr_parse_long_long(const uint8_t* begin, const uint8_t* end, long long* data);

/**
 * \brief Parses an unsigned long integer in the requested base.
 *
 * \param begin First byte of the input range.
 * \param end One past the last byte of the input range.
 * \param base Numeric base accepted by strtoul(), including zero for automatic
 * base detection.
 * \param data Receives the parsed value.
 * \return A pointer after the parsed prefix, begin if no conversion is
 * possible, or NULL if conversion does not produce a valid position.
 */
const uint8_t *bstr_parse_unsigned_long(const uint8_t *begin, const uint8_t *end, uint8_t base, unsigned long *data);

/**
 * \brief Parses an unsigned long long integer in the requested base.
 *
 * \param begin First byte of the input range.
 * \param end One past the last byte of the input range.
 * \param base Numeric base accepted by strtoull(), including zero for automatic
 * base detection.
 * \param data Receives the parsed value.
 * \return A pointer after the parsed prefix, begin if no conversion is
 * possible, or NULL if conversion does not produce a valid position.
 */
const uint8_t* bstr_parse_unsigned_long_long(const uint8_t* begin, const uint8_t* end, uint8_t base, unsigned long long* data);

/**
 * \brief Parses the integer component of a JSON number.
 *
 * Fraction and exponent components are not currently parsed.
 *
 * \param ctx Context that receives parser error information.
 * \param begin First byte of the input range.
 * \param end One past the last byte of the input range.
 * \param number Receives the parsed number fields.
 * \return A pointer after the parsed integer component, or NULL on error.
 */
const uint8_t *bstr_parse_json_number(bstr_context_t *ctx, const uint8_t *begin, const uint8_t *end, bstr_number_t *number);

/**
 * \brief Parses and decodes an RFC 8259 JSON string literal.
 *
 * Decoded content is appended to str only after a complete valid literal has
 * been parsed. JSON escapes, UTF-16 surrogate pairs, and raw UTF-8 are handled.
 *
 * \param ctx Context that receives parser error information.
 * \param begin First byte of the input range; it must be a quotation mark.
 * \param end One past the last byte available for parsing.
 * \param str String to which the decoded UTF-8 content is appended.
 * \return A pointer immediately after the closing quotation mark, or NULL on
 * invalid input, premature end of input, or allocation failure.
 */
const uint8_t *bstr_parse_json_string_literal(bstr_context_t *ctx, const uint8_t *begin, const uint8_t *end, adt_str_t *str);

/**
 * \brief Finds the first line-feed byte in a bounded range.
 *
 * \param begin First byte of the range.
 * \param end One past the last byte of the range.
 * \return A pointer to the first line feed, end if none is found, or NULL if
 * begin is after end.
 */
const uint8_t *bstr_find_line_feed(const uint8_t *begin, const uint8_t *end);

/**
 * \brief Advances through a range while a predicate is true.
 *
 * \param begin First byte of the range.
 * \param end One past the last byte of the range.
 * \param predicate Function used to test each byte.
 * \return The first position for which predicate is false, or end if every
 * byte matches.
 */
const uint8_t *bstr_skip_forward_while(const uint8_t *begin, const uint8_t *end, int (*predicate)(int c));

/**
 * \brief Moves backward from the end of a range while a predicate is true.
 *
 * \param begin First byte of the range.
 * \param end One past the last byte of the range.
 * \param predicate Function used to test each byte.
 * \return The new exclusive end position after matching suffix bytes have been
 * skipped, or begin if the entire range matches or the range is empty.
 */
const uint8_t *bstr_skip_backward_while(const uint8_t *begin, const uint8_t *end, int (*predicate)(int c) );

/**
 * \brief Skips whitespace at the beginning of a bounded range.
 *
 * \param begin First byte of the range.
 * \param end One past the last byte of the range.
 * \return The first non-whitespace position, or end if the range contains only
 * whitespace.
 */
const uint8_t *bstr_lstrip(const uint8_t *begin, const uint8_t *end);

/**
 * \brief Skips whitespace at the end of a bounded range.
 *
 * \param begin First byte of the range.
 * \param end One past the last byte of the range.
 * \return The new exclusive end position, or begin if the range contains only
 * whitespace.
 */
const uint8_t *bstr_rstrip(const uint8_t *begin, const uint8_t *end);

/**
 * \brief Finds the range remaining after leading and trailing whitespace.
 *
 * \param begin First byte of the input range.
 * \param end One past the last byte of the input range.
 * \param stripped_begin Receives the first non-whitespace position.
 * \param stripped_end Receives the exclusive end after trailing whitespace.
 */
void bstr_strip(const uint8_t *begin, const uint8_t *end, const uint8_t **stripped_begin, const uint8_t **stripped_end);

/*************** predicate functions ***************/

/**
 * \brief Tests whether a value is an ASCII space or horizontal tab.
 * \param c Value to test.
 * \return Nonzero if c is horizontal whitespace; otherwise zero.
 */
int bstr_pred_is_horizontal_space(int c);

/**
 * \brief Tests whether a value is JSON whitespace.
 * \param c Value to test.
 * \return Nonzero for space, horizontal tab, line feed, or carriage return;
 * otherwise zero.
 */
int bstr_pred_is_whitespace(int c);

/**
 * \brief Tests whether a value is an ASCII decimal digit.
 * \param c Value to test.
 * \return Nonzero for values from '0' through '9'; otherwise zero.
 */
int bstr_pred_is_digit(int c);

/**
 * \brief Tests whether a value is an ASCII hexadecimal digit.
 * \param c Value to test.
 * \return Nonzero for decimal digits or letters A-F and a-f; otherwise zero.
 */
int bstr_pred_is_hex_digit(int c);

/**
 * \brief Tests whether a value is an ASCII digit from one through nine.
 * \param c Value to test.
 * \return Nonzero for values from '1' through '9'; otherwise zero.
 */
int bstr_pred_is_nonzero_digit(int c);

/**
 * \brief Tests whether a value is an ASCII control character.
 * \param c Value to test.
 * \return Nonzero if c is less than 0x20; otherwise zero.
 */
int bstr_pred_is_control_char(int c);

#endif //BSTR_H