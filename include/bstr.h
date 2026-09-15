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
void bstr_context_create(bstr_context_t *self);
bstr_context_t *bstr_context_new(void);
void bstr_context_delete(bstr_context_t *self);
char* bstr_make_cstr(const uint8_t *begin, const uint8_t *end);
char* bstr_make_cstr_x(const uint8_t *begin, const uint8_t *end, uint16_t start_offset, uint16_t end_offset);
const uint8_t *bstr_search_val(const uint8_t *begin, const uint8_t *end, uint8_t val);
const uint8_t *bstr_match_pair(const uint8_t *begin, const uint8_t *end, uint8_t left, uint8_t right, uint8_t escape_char);
const uint8_t *bstr_match_bstr(const uint8_t *begin, const uint8_t *end, const uint8_t *str_begin, const uint8_t *str_end);
const uint8_t *bstr_match_cstr(const uint8_t *begin, const uint8_t *end, const char *cstr);
const uint8_t* bstr_to_double(const uint8_t* begin, const uint8_t* end, double* data);
const uint8_t *bstr_to_long(const uint8_t *begin, const uint8_t *end, long *data);
const uint8_t* bstr_to_long_long(const uint8_t* begin, const uint8_t* end, long long* data);
const uint8_t *bstr_to_unsigned_long(const uint8_t *begin, const uint8_t *end, uint8_t base, unsigned long *data);
const uint8_t* bstr_to_unsigned_long_long(const uint8_t* begin, const uint8_t* end, uint8_t base, unsigned long long* data);
const uint8_t *bstr_parse_json_number(bstr_context_t *ctx, const uint8_t *begin, const uint8_t *end, bstr_number_t *number);
const uint8_t *bstr_parse_json_string_literal(bstr_context_t *ctx, const uint8_t *begin, const uint8_t *end, adt_str_t *str);
const uint8_t *bstr_line(const uint8_t *begin, const uint8_t *end);
const uint8_t *bstr_skip_forward_while(const uint8_t *begin, const uint8_t *end, int (*predicate)(int c));
const uint8_t *bstr_skip_backward_while(const uint8_t *begin, const uint8_t *end, int (*predicate)(int c) );
const uint8_t *bstr_lstrip(const uint8_t *begin, const uint8_t *end);
const uint8_t *bstr_rstrip(const uint8_t *begin, const uint8_t *end);
void bstr_strip(const uint8_t *begin, const uint8_t *end, const uint8_t **stripped_begin, const uint8_t **stripped_end);
bstr_error_t bstr_get_last_error(bstr_context_t *ctx);
void bstr_clear_error(bstr_context_t *ctx);

/*************** predicate functions ***************/
int bstr_pred_is_horizontal_space(int c);
int bstr_pred_is_whitespace(int c);
int bstr_pred_is_digit(int c);
int bstr_pred_is_hex_digit(int c);
int bstr_pred_is_one_nine(int c);
int bstr_pred_is_control_char(int c);
int bstr_pred_is_not_zero(int c);

#endif //BSTR_H