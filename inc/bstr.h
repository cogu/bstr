/*****************************************************************************
* \file      bstr.h
* \author    Conny Gustafsson
* \date      2017-08-04
* \brief     Bounded strings library
*
* Copyright (c) 2017-2019 Conny Gustafsson
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in
* the Software without restriction, including without limitation the rights to
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
* the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
******************************************************************************/
#ifndef BSTR_H
#define BSTR_H

//////////////////////////////////////////////////////////////////////////////
// MODULE VERSION
//////////////////////////////////////////////////////////////////////////////

#define _xstr(s) _str(s)
#define _str(s) #s
#define _MAKE_VERSION_STR(x, y, z) _xstr(x) "." _xstr(y) "." _xstr(z)

#define BSTR_VERSION_MAJOR 0
#define BSTR_VERSION_MINOR 1
#define BSTR_VERSION_PATCH 0

#define BSTR_VERSION _MAKE_VERSION_STR(BSTR_VERSION_MAJOR, BSTR_VERSION_MINOR, BSTR_VERSION_PATCH)

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
   bool hasInteger;
   bool hasFraction;
   bool hasExponent;
   bool isNegative;
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
   bstr_error_t lastError;
} bstr_context_t;


//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
void bstr_context_create(bstr_context_t *self);
bstr_context_t *bstr_context_new(void);
void bstr_context_delete(bstr_context_t *self);
char* bstr_make_cstr(const uint8_t *pBegin, const uint8_t *pEnd);
char* bstr_make_x_cstr(const uint8_t *pBegin, const uint8_t *pEnd, uint16_t startOffset, uint16_t endOffset);
const uint8_t *bstr_search_val(const uint8_t *pBegin, const uint8_t *pEnd, uint8_t val);
const uint8_t *bstr_match_pair(const uint8_t *pBegin, const uint8_t *pEnd, uint8_t left, uint8_t right, uint8_t escapeChar);
const uint8_t *bstr_match_bstr(const uint8_t *pBegin, const uint8_t *pEnd,const uint8_t *pStrBegin, const uint8_t *pStrEnd);
const uint8_t *bstr_match_cstr(const uint8_t *pBegin, const uint8_t *pEnd, const char *cstr);
const uint8_t *bstr_to_long(const uint8_t *pBegin, const uint8_t *pEnd, long *data);
const uint8_t *bstr_to_unsigned_long(const uint8_t *pBegin, const uint8_t *pEnd, uint8_t base, unsigned long *data);
const uint8_t *bstr_parse_json_number(bstr_context_t *ctx, const uint8_t *pBegin, const uint8_t *pEnd, bstr_number_t *number);
const uint8_t *bstr_parse_json_string_literal(bstr_context_t *ctx, const uint8_t *pBegin, const uint8_t *pEnd, adt_str_t *str);
const uint8_t *bstr_line(const uint8_t *pBegin, const uint8_t *pEnd);
const uint8_t *bstr_while_predicate(const uint8_t *pBegin, const uint8_t *pEnd, int (*pred)(int c));
const uint8_t *bstr_while_predicate_reverse(const uint8_t *pBegin, const uint8_t *pEnd, int (*pred_func)(int c) );
const uint8_t *bstr_lstrip(const uint8_t *pBegin, const uint8_t *pEnd);
const uint8_t *bstr_rstrip(const uint8_t *pBegin, const uint8_t *pEnd);
void bstr_strip(const uint8_t *pBegin, const uint8_t *pEnd, const uint8_t **strippedBegin, const uint8_t **strippedEnd);
bstr_error_t bstr_get_last_error(bstr_context_t *ctx);
void bstr_clear_error(bstr_context_t *ctx);

/*************** predicate functions ***************/
int bstr_pred_is_horizontal_space(int c);
int bstr_pred_is_whitespace(int c);
int bstr_pred_is_digit(int c);
int bstr_pred_is_hex_digit(int c);
int bstr_pred_is_one_nine(int c);
int bstr_pred_is_control_char(int c);

#endif //BSTR_H
