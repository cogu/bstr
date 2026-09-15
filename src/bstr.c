/*****************************************************************************
* \file      bstr.c
* \author    Conny Gustafsson
* \date      2017-08-04
* \brief     Bounded strings library
*
* Copyright (c) 2017-2026 Conny Gustafsson
* SPDX-License-Identifier: MIT
* See LICENSE in project root for full license terms.
******************************************************************************/
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include "bstr.h"

#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define MAX_NUMBER_SIZE 32

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void bstr_set_error(bstr_context_t *ctx, bstr_error_t error_code);
static const uint8_t *bstr_parse_number_int(bstr_context_t *ctx, const uint8_t *begin, const uint8_t *end, bstr_number_t *number);
static adt_error_t bstr_append_code_point(adt_str_t *str, uint32_t code_point);
static int32_t bstr_utf8_sequence_size(const uint8_t *begin, const uint8_t *end);

//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////
static const int ASCIIHexToInt[256] =
{
    // ASCII
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

void bstr_context_create(bstr_context_t *self)
{
   if (self != NULL)
   {
      self->last_error = BSTR_NO_ERROR;
   }
}

bstr_context_t *bstr_context_new(void)
{
   bstr_context_t *self = (bstr_context_t*) malloc(sizeof(bstr_context_t));
   if (self != NULL)
   {
      bstr_context_create(self);
   }
   return self;
}

void bstr_context_delete(bstr_context_t *self)
{
   if (self != NULL)
   {
      free(self);
   }
}

bstr_error_t bstr_context_last_error(bstr_context_t *ctx)
{
   return ctx->last_error;
}

void bstr_context_clear_error(bstr_context_t *ctx)
{
   ctx->last_error = BSTR_NO_ERROR;
}


char* bstr_make_cstr(const uint8_t *begin, const uint8_t *end){
   if( (begin != NULL) && (end != NULL) && (begin<end)){
      uint32_t len = (uint32_t) (end-begin);
      uint8_t *str = (uint8_t*) malloc(len+1);
      if(str != NULL){
         memcpy(str,begin,len);
         str[len]=0;
      }
      return (char*) str;
   }
   return NULL;
}

char *bstr_make_cstr_with_padding(const uint8_t *begin, const uint8_t *end, uint16_t padding_left, uint16_t padding_right){
   if( (begin != NULL) && (end != NULL) && (begin)){
      uint8_t *str;
      uint32_t allocLen;
      uint32_t strLen = (uint32_t) (end-begin);
      allocLen = strLen+padding_left+padding_right+1;
      str = (uint8_t*) malloc(allocLen);
      if(str != NULL){
         memcpy(str+padding_left,begin,strLen);
         str[allocLen-1]=(uint8_t)0;
      }
      return (char*)str;
   }
   return NULL;
}

const uint8_t *bstr_find_byte(const uint8_t *begin, const uint8_t *end, uint8_t byte){
   const uint8_t *next = begin;
   if (next > end)
   {
      return NULL; //invalid arguments
   }
   while(next < end){
      uint8_t c = *next;
      if(c == byte){
         return next;
      }
      next++;
   }
   return end;
}

const uint8_t *bstr_match_pair(const uint8_t *begin, const uint8_t *end, uint8_t left, uint8_t right, uint8_t escape_char){
   const uint8_t *next = begin;
   uint32_t innerLevelCount=0;
   if (next < end){
      if (*next == left){
         next++;
         if (escape_char != 0){
            uint8_t isEscape = 0;
            while (next < end){
               uint8_t c = *next;
               if (isEscape != 0){
                  //ignore this char
                  next++;
                  isEscape = 0;
                  continue;
               }
               else {
                  if ( c == escape_char ){
                     isEscape = 1;
                  }
                  else if (c == right){
                     if (innerLevelCount == 0) {
                        return next;
                     }
                     else {
                        innerLevelCount--;
                     }
                  }
                  else if ( c == left )
                  {
                     innerLevelCount++;
                  }
               }
               next++;
            }
         }
         else{
            while (next < end) {
               uint8_t c = *next;
               if (c == right){
                  if (innerLevelCount == 0) {
                     return next;
                  }
                  else {
                     innerLevelCount--;
                  }
               }
               else if (c == left)
               {
                  innerLevelCount++;
               }
               next++;
            }
         }
      }
      else
      {
         return NULL; //string does not start with \par left character
      }
   }
   return begin;
}

const uint8_t *bstr_match_bstr(const uint8_t *begin, const uint8_t *end, const uint8_t *str_begin, const uint8_t *str_end)
{
   const uint8_t *next = begin;
   const uint8_t *str_next = str_begin;
   if ( (begin > end) || (str_begin > str_end) )
   {
      return NULL;
   }
   while(next < end){
      if (str_next < str_end)
      {
         if (*next != *str_next)
         {
            return NULL; //string did not match
         }
      }
      else
      {
         //All characters in str_begin has been successfully matched
         return next; //next should point to str_end at this point
      }
      next++;
      str_next++;
   }
   if (str_next == str_end)
   {
      return next; //All characters in str_begin has been successfully matched
   }
   return begin; //reached end before str_begin was fully matched
}

const uint8_t *bstr_match_cstr(const uint8_t *begin, const uint8_t *end, const char *cstr)
{
   const uint8_t *str_begin = (const uint8_t*) cstr;
   const uint8_t *str_end;
   if ( (begin == NULL) || (end == NULL) || (end < begin) || (cstr == NULL) )
   {
      return NULL;
   }
   str_end = str_begin + strlen(cstr);
   return bstr_match_bstr(begin, end, str_begin, str_end);
}

const uint8_t* bstr_parse_double(const uint8_t* begin, const uint8_t* end, double* data)
{
   char tmp[MAX_NUMBER_SIZE+1];   
   char* parse_end = NULL;
   size_t size = end - begin;
   if (size > MAX_NUMBER_SIZE)
   {
      size = MAX_NUMBER_SIZE;
   }
   memcpy(&tmp[0], begin, size);
   tmp[size]='\0';
   *data = strtod(&tmp[0], &parse_end);
   if (parse_end > &tmp[0] )
   {
      size_t delta = parse_end - &tmp[0];
      const uint8_t* retval = begin + delta;
      if (retval <= end)
      {
         return retval;
      }   
   }
   else if (parse_end == &tmp[0])
   {
      return begin; //Not a number
   }
   return NULL;
}

const uint8_t *bstr_parse_long(const uint8_t *begin, const uint8_t *end, long *data)
{
   char tmp[MAX_NUMBER_SIZE+1];   
   char* parse_end = NULL;
   size_t size = end - begin;
   if (size > MAX_NUMBER_SIZE)
   {
      size = MAX_NUMBER_SIZE;
   }
   memcpy(&tmp[0], begin, size);
   tmp[size]='\0';
   *data = strtol(&tmp[0], &parse_end, 0);   
   if (parse_end > &tmp[0] )
   {
      size_t delta = parse_end - &tmp[0];
      const uint8_t* retval = begin + delta;
      if (retval <= end)
      {
         return retval;
      }   
   }
   else if (parse_end == &tmp[0])
   {
      return begin; //Not a number
   }
   return NULL;
}

const uint8_t* bstr_parse_long_long(const uint8_t* begin, const uint8_t* end, long long* data)
{
   char tmp[MAX_NUMBER_SIZE+1];   
   char* parse_end = NULL;
   size_t size = end - begin;
   if (size > MAX_NUMBER_SIZE)
   {
      size = MAX_NUMBER_SIZE;
   }
   memcpy(&tmp[0], begin, size);
   tmp[size]='\0';
   *data = strtoll(&tmp[0], &parse_end, 0);   
   if (parse_end > &tmp[0])
   {
      size_t delta = parse_end - &tmp[0];
      const uint8_t* retval = begin + delta;
      if (retval <= end)
      {
         return retval;
      }
   }
   else if (parse_end == &tmp[0])
   {
      return begin; //Not a number
   }
   return NULL;
}


const uint8_t *bstr_parse_unsigned_long(const uint8_t *begin, const uint8_t *end, uint8_t base, unsigned long *data)
{
   char tmp[MAX_NUMBER_SIZE+1];   
   char* parse_end = NULL;
   size_t size = end - begin;
   if (size > MAX_NUMBER_SIZE)
   {
      size = MAX_NUMBER_SIZE;
   }
   memcpy(&tmp[0], begin, size);
   tmp[size]='\0';
   *data = strtoul(&tmp[0], &parse_end, base);   
   if (parse_end > &tmp[0] )
   {
      size_t delta = parse_end - &tmp[0];
      const uint8_t* retval = begin + delta;
      if (retval <= end)
      {
         return retval;
      }
   }
   else if (parse_end == &tmp[0])
   {
      return begin; //Not a number
   }
   return NULL;
}

const uint8_t* bstr_parse_unsigned_long_long(const uint8_t* begin, const uint8_t* end, uint8_t base, unsigned long long* data)
{
   char tmp[MAX_NUMBER_SIZE+1];   
   char* parse_end = NULL;
   size_t size = end - begin;
   if (size > MAX_NUMBER_SIZE)
   {
      size = MAX_NUMBER_SIZE;
   }
   memcpy(&tmp[0], begin, size);
   tmp[size]='\0';
   *data = strtoull(&tmp[0], &parse_end, base);   
   if (parse_end > &tmp[0] )
   {
      size_t delta = parse_end - &tmp[0];
      const uint8_t* retval = begin + delta;
      if (retval <= end)
      {
         return retval;
      }
   }
   else if (parse_end == &tmp[0])
   {
      return begin; //Not a number
   }
   return NULL;
}

const uint8_t *bstr_parse_json_number(bstr_context_t *ctx, const uint8_t *begin, const uint8_t *end, bstr_number_t *number)
{
   const uint8_t *result;
   const uint8_t *next = begin;
   if ( (ctx == NULL) || (begin == NULL) || (end == NULL) || (number == NULL) || (begin > end) )
   {
      return NULL;
   }
   number->has_integer = false;
   number->has_fraction = false;
   number->has_exponent = false;
   number->is_negative = false;
   if (next < end)
   {
      result = bstr_parse_number_int(ctx, next, end, number);
      next = result;
   }
   else
   {
      //empty string
   }
   return next;
}

const uint8_t *bstr_parse_json_string_literal(bstr_context_t *ctx, const uint8_t *begin, const uint8_t *end, adt_str_t *str)
{
   if ( (ctx == NULL) || (begin == NULL) || (end == NULL) || (str == NULL) || (end < begin) )
   {
      return NULL;
   }

   if (begin == end)
   {
      bstr_set_error(ctx, BSTR_PREMATURE_END_OF_BUFFER_ERROR);
      return NULL;
   }
   if (*begin != '"')
   {
      bstr_set_error(ctx, BSTR_INVALID_CHARACTER_ERROR);
      return NULL;
   }

   adt_str_t parsed;
   const uint8_t *next = begin + 1;
   bstr_error_t error = BSTR_PREMATURE_END_OF_BUFFER_ERROR;
   adt_str_create(&parsed);

   while (next < end)
   {
      uint8_t c = *next++;
      adt_error_t result;

      if (c == '"')
      {
         result = adt_str_append(str, &parsed);
         adt_str_destroy(&parsed);
         if (result != ADT_NO_ERROR)
         {
            bstr_set_error(ctx, BSTR_MEM_ERROR);
            return NULL;
         }
         return next;
      }

      if (c == '\\')
      {
         uint8_t escaped;
         if (next == end)
         {
            break;
         }
         escaped = *next++;
         switch (escaped)
         {
         case '"': c = '"'; break;
         case '\\': c = '\\'; break;
         case '/': c = '/'; break;
         case 'b': c = '\b'; break;
         case 'f': c = '\f'; break;
         case 'n': c = '\n'; break;
         case 'r': c = '\r'; break;
         case 't': c = '\t'; break;
         case 'u':
         {
            uint32_t code_point = 0u;
            uint32_t digit;
            int32_t index;

            if ((end - next) < 4)
            {
               goto parse_error;
            }
            for (index = 0; index < 4; index++)
            {
               int value = ASCIIHexToInt[next[index]];
               if (value < 0)
               {
                  error = BSTR_INVALID_CHARACTER_ERROR;
                  goto parse_error;
               }
               digit = (uint32_t) value;
               code_point = (code_point << 4) | digit;
            }
            next += 4;

            if ((code_point >= 0xd800u) && (code_point <= 0xdbffu))
            {
               uint32_t low_surrogate = 0u;
               if ((end - next) < 6)
               {
                  if ((next < end) && (*next == '"'))
                  {
                     error = BSTR_INVALID_CHARACTER_ERROR;
                  }
                  goto parse_error;
               }
               if ((next[0] != '\\') || (next[1] != 'u'))
               {
                  error = BSTR_INVALID_CHARACTER_ERROR;
                  goto parse_error;
               }
               next += 2;
               for (index = 0; index < 4; index++)
               {
                  int value = ASCIIHexToInt[next[index]];
                  if (value < 0)
                  {
                     error = BSTR_INVALID_CHARACTER_ERROR;
                     goto parse_error;
                  }
                  digit = (uint32_t) value;
                  low_surrogate = (low_surrogate << 4) | digit;
               }
               next += 4;
               if ((low_surrogate < 0xdc00u) || (low_surrogate > 0xdfffu))
               {
                  error = BSTR_INVALID_CHARACTER_ERROR;
                  goto parse_error;
               }
               code_point = 0x10000u + ((code_point - 0xd800u) << 10)
                     + (low_surrogate - 0xdc00u);
            }
            else if ((code_point >= 0xdc00u) && (code_point <= 0xdfffu))
            {
               error = BSTR_INVALID_CHARACTER_ERROR;
               goto parse_error;
            }

            result = bstr_append_code_point(&parsed, code_point);
            if (result != ADT_NO_ERROR)
            {
               error = BSTR_MEM_ERROR;
               goto parse_error;
            }
            continue;
         }
         default:
            error = BSTR_INVALID_CHARACTER_ERROR;
            goto parse_error;
         }

         result = adt_str_push(&parsed, c);
         if (result != ADT_NO_ERROR)
         {
            error = BSTR_MEM_ERROR;
            goto parse_error;
         }
      }
      else if (bstr_pred_is_control_char(c))
      {
         error = BSTR_INVALID_CHARACTER_ERROR;
         goto parse_error;
      }
      else if (c < 0x80u)
      {
         result = adt_str_push(&parsed, c);
         if (result != ADT_NO_ERROR)
         {
            error = BSTR_MEM_ERROR;
            goto parse_error;
         }
      }
      else
      {
         const uint8_t *sequence_begin = next - 1;
         int32_t sequence_size = bstr_utf8_sequence_size(sequence_begin, end);
         if (sequence_size < 0)
         {
            goto parse_error;
         }
         if (sequence_size == 0)
         {
            error = BSTR_INVALID_CHARACTER_ERROR;
            goto parse_error;
         }
         result = adt_str_append_bstr(&parsed, sequence_begin, sequence_begin + sequence_size);
         if (result != ADT_NO_ERROR)
         {
            error = BSTR_MEM_ERROR;
            goto parse_error;
         }
         next = sequence_begin + sequence_size;
      }
   }

parse_error:
   adt_str_destroy(&parsed);
   bstr_set_error(ctx, error);
   return NULL;
}

const uint8_t *bstr_find_line_feed(const uint8_t *begin, const uint8_t *end)
{
   return bstr_find_byte(begin, end, (uint8_t) '\n');
}

const uint8_t *bstr_skip_forward_while(const uint8_t *begin, const uint8_t *end, int (*predicate)(int c) )
{
   const uint8_t *next = begin;
   while (next < end)
   {
      int c = (int) *next;
      if (!predicate(c)){
         break;
      }
      next++;
   }
   return next;
}

const uint8_t *bstr_skip_backward_while(const uint8_t *begin, const uint8_t *end, int (*predicate)(int c) )
{
   if (begin < end)
   {
      const uint8_t *next = end;
      while (next > begin)
      {
         const uint8_t *test = next-1;
         int c = (int) *test;
         if (!predicate(c)){
            break;
         }
         next--;
      }
      return next;
   }
   return begin;
}

const uint8_t *bstr_lstrip(const uint8_t *begin, const uint8_t *end)
{
   return bstr_skip_forward_while(begin, end, bstr_pred_is_whitespace);
}

const uint8_t *bstr_rstrip(const uint8_t *begin, const uint8_t *end)
{
   return bstr_skip_backward_while(begin, end, bstr_pred_is_whitespace);
}

void bstr_strip(const uint8_t *begin, const uint8_t *end, const uint8_t **stripped_begin, const uint8_t **stripped_end)
{
   *stripped_begin = bstr_lstrip(begin, end);
   *stripped_end = bstr_rstrip(*stripped_begin, end);
}

/*************** predicate functions ***************/
int bstr_pred_is_horizontal_space(int c)
{
   return (c == (int) '\t') || (c == (int) ' ');
}

int bstr_pred_is_whitespace(int c)
{
   return (c == (int) '\t') || (c == (int) '\n') || (c == (int) '\r') || (c == (int) ' ');
}

int bstr_pred_is_digit(int c)
{
   return (c >= '0') && (c <= '9');
}

int bstr_pred_is_hex_digit(int c)
{
   return ((c >= '0') && (c <= '9') ) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'));
}

int bstr_pred_is_nonzero_digit(int c)
{
   return (c >= '1') && (c <= '9');
}

int bstr_pred_is_control_char(int c)
{
   return (c < 32);
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static adt_error_t bstr_append_code_point(adt_str_t *str, uint32_t code_point)
{
   uint8_t encoded[4];
   int32_t size;

   if (code_point <= 0x7fu)
   {
      return adt_str_push(str, (int) code_point);
   }
   if (code_point <= 0x7ffu)
   {
      encoded[0] = (uint8_t) (0xc0u | (code_point >> 6));
      encoded[1] = (uint8_t) (0x80u | (code_point & 0x3fu));
      size = 2;
   }
   else if (code_point <= 0xffffu)
   {
      encoded[0] = (uint8_t) (0xe0u | (code_point >> 12));
      encoded[1] = (uint8_t) (0x80u | ((code_point >> 6) & 0x3fu));
      encoded[2] = (uint8_t) (0x80u | (code_point & 0x3fu));
      size = 3;
   }
   else
   {
      encoded[0] = (uint8_t) (0xf0u | (code_point >> 18));
      encoded[1] = (uint8_t) (0x80u | ((code_point >> 12) & 0x3fu));
      encoded[2] = (uint8_t) (0x80u | ((code_point >> 6) & 0x3fu));
      encoded[3] = (uint8_t) (0x80u | (code_point & 0x3fu));
      size = 4;
   }
   return adt_str_append_bstr(str, encoded, encoded + size);
}

static int32_t bstr_utf8_sequence_size(const uint8_t *begin, const uint8_t *end)
{
   const uint8_t lead = begin[0];
   int32_t size;
   int32_t index;

   if ((lead >= 0xc2u) && (lead <= 0xdfu))
   {
      size = 2;
   }
   else if ((lead >= 0xe0u) && (lead <= 0xefu))
   {
      size = 3;
   }
   else if ((lead >= 0xf0u) && (lead <= 0xf4u))
   {
      size = 4;
   }
   else
   {
      return 0;
   }

   if ((end - begin) < size)
   {
      return -1;
   }
   for (index = 1; index < size; index++)
   {
      if ((begin[index] & 0xc0u) != 0x80u)
      {
         return 0;
      }
   }
   if (((lead == 0xe0u) && (begin[1] < 0xa0u)) ||
       ((lead == 0xedu) && (begin[1] > 0x9fu)) ||
       ((lead == 0xf0u) && (begin[1] < 0x90u)) ||
       ((lead == 0xf4u) && (begin[1] > 0x8fu)))
   {
      return 0;
   }
   return size;
}

void bstr_set_error(bstr_context_t *ctx, bstr_error_t error_code)
{
   ctx->last_error = error_code;
}

const uint8_t *bstr_parse_number_int(bstr_context_t *ctx, const uint8_t *begin, const uint8_t *end, bstr_number_t *number)
{
   const uint8_t *next = begin;
   const int32_t base = 10;

   if (next < end)
   {
      char c =  (char) *next;
      if (c == '-')
      {
         number->is_negative = true;
         ++next;
         if (next < end)
         {
            c =  (char) *next;
         }
         else
         {
            bstr_set_error(ctx, BSTR_PARSE_ERROR);
            return NULL;
         }
      }
      if (c == '0')
      {
         number->integer = 0;
         number->has_integer = true;
         ++next;
      }
      else if (bstr_pred_is_nonzero_digit(c))
      {
         int64_t intPart = ASCIIHexToInt[(int) c];
         next++;
         while(next < end)
         {
            int tmp =  (int) *next;
            if (bstr_pred_is_digit(tmp))
            {
               intPart *= base;
               intPart += ASCIIHexToInt[tmp];
               if (intPart > UINT32_MAX)
               {
                  bstr_set_error(ctx, BSTR_NUMBER_TOO_LARGE_ERROR);
                  return NULL;
               }
               next++;
            }
            else
            {
               break; //possible start of fraction
            }
         }
         number->has_integer = true;
         number->integer = (uint32_t) intPart;
      }
      else
      {
         bstr_set_error(ctx, BSTR_PARSE_ERROR);
         next = 0;
      }
   }
   return next;
}

