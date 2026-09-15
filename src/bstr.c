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
#include <errno.h>
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

/**
 * Creates new bstr context (for purposes of thread safety)
 */
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


/**
 * Similar to strdup but operates on a bounded string. Returns a new NULL-terminated C string.
 * Additionally the caller is responsible for freeing up the memory allocated by this function
 * by calling free on the returned pointer.
 */
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

/**
 * Similar to bstr_make but in addition it adds optional space before and after the copied string.
 * start_offset is the number of extra bytes to add before the (copied) string while
 * end_offset is the number of extra bytes to add after string.
 * It's OK to set one of the offsets to zero. If both start_offset and end_offset are zero
 * it behaves identical to calling bstr_make_cstr directly
 */
char *bstr_make_cstr_x(const uint8_t *begin, const uint8_t *end, uint16_t start_offset, uint16_t end_offset){
   if( (begin != NULL) && (end != NULL) && (begin)){
      uint8_t *str;
      uint32_t allocLen;
      uint32_t strLen = (uint32_t) (end-begin);
      allocLen = strLen+start_offset+end_offset+1;
      str = (uint8_t*) malloc(allocLen);
      if(str != NULL){
         memcpy(str+start_offset,begin,strLen);
         str[allocLen-1]=(uint8_t)0;
      }
      return (char*)str;
   }
   return NULL;
}

/**
 * scans for \par val between \par begin and \par end.
 * On success it returns the pointer to \par val.
 * On failure it returns \par begin if not found or NULL if invalid arguments was given.
 */
const uint8_t *bstr_search_val(const uint8_t *begin, const uint8_t *end, uint8_t val){
   const uint8_t *next = begin;
   if (next > end)
   {
      return NULL; //invalid arguments
   }
   while(next < end){
      uint8_t c = *next;
      if(c == val){
         return next;
      }
      next++;
   }
   return begin; //val was not found before end was reached
}

/**
 * scans for matching \par left and \par right characters in a string. Used for matching '(' with ')', '[' with, ']' etc.
 * On Success it returns the pointer to \par right.
 * On failure it returns \par begin if the scan reached \par end before \par right was found.
 * If it cannot even match \par left on the first character of \par begin it returns NULL.
 */
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

/**
 * \brief compares characters in string bounded by str_begin and str_end in buffer bound by begin and end
 * \param begin start of buffer
 * \param end end of buffer
 * \param str_begin start of string to be matched
 * \param str_end end of string to matched
 * \return On success, pointer in buffer where the match stopped. On match failure it returns 0. If end was reached before str_end was fully matched it returns begin.
 */
const uint8_t *bstr_match_bstr(const uint8_t *begin, const uint8_t *end, const uint8_t *str_begin, const uint8_t *str_end)
{
   const uint8_t *next = begin;
   const uint8_t *str_next = str_begin;
   if ( (begin > end) || (str_begin > str_end) )
   {
      errno = EINVAL; //invalid arguments
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

/**
 * Checks if the C string (cstr) is a substring of the bounded string (bstr).
 */
const uint8_t *bstr_match_cstr(const uint8_t *begin, const uint8_t *end, const char *cstr)
{
   const uint8_t *str_begin = (const uint8_t*) cstr;
   const uint8_t *str_end;
   if ( (begin == NULL) || (end == NULL) || (end < begin) || (cstr == NULL) )
   {
      errno = EINVAL; //invalid arguments
      return NULL;
   }
   str_end = str_begin + strlen(cstr);
   return bstr_match_bstr(begin, end, str_begin, str_end);
}

const uint8_t* bstr_to_double(const uint8_t* begin, const uint8_t* end, double* data)
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

const uint8_t *bstr_to_long(const uint8_t *begin, const uint8_t *end, long *data)
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

const uint8_t* bstr_to_long_long(const uint8_t* begin, const uint8_t* end, long long* data)
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


const uint8_t *bstr_to_unsigned_long(const uint8_t *begin, const uint8_t *end, uint8_t base, unsigned long *data)
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

const uint8_t* bstr_to_unsigned_long_long(const uint8_t* begin, const uint8_t* end, uint8_t base, unsigned long long* data)
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

/**
 * Parses a number from a bounded string using JSON number format
 */
const uint8_t *bstr_parse_json_number(bstr_context_t *ctx, const uint8_t *begin, const uint8_t *end, bstr_number_t *number)
{
   const uint8_t *result;
   const uint8_t *next = begin;
   if ( (ctx == NULL) || (begin == NULL) || (end == NULL) || (number == NULL) || (begin > end) )
   {
      errno = EINVAL; //invalid arguments
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

/**
 * Using the JSON definition, this function parses a double-quoted string literal.
 * The parsed string (not including the the quotation marks) will be stored in the str parameter
 */
const uint8_t *bstr_parse_json_string_literal(bstr_context_t *ctx, const uint8_t *begin, const uint8_t *end, adt_str_t *str)
{
#define NUM_ESCAPE_CHARS 8
   const uint8_t quotationMark = '"';
   const uint8_t backslash = '\\';
   const uint8_t frontslash = '/';
   const uint8_t backspace = '\b';
   const uint8_t formfeed = '\f';
   const uint8_t linefeed = '\n';
   const uint8_t carriageReturn = '\r';
   const uint8_t horizontalTab = '\t';
   const uint8_t validEscapeChars[NUM_ESCAPE_CHARS] = {
         quotationMark,
         backslash,
         frontslash,
         'b',
         'f',
         'n',
         'r',
         't'
   };
   const uint8_t escapeCharMap[NUM_ESCAPE_CHARS] = {
         quotationMark,
         backslash,
         frontslash,
         backspace,
         formfeed,
         linefeed,
         carriageReturn,
         horizontalTab,
   };

   if ( (ctx == NULL) || (begin == NULL) || (end == NULL) || (str == NULL) || (end < begin) )
   {
      errno = EINVAL;
      return NULL;
   }
   if (begin < end)
   {
      uint8_t firstChar = *begin;
      if (firstChar == quotationMark)
      {
         const uint8_t *next = begin+1;
         bool isEscapeSequence = false;
         uint8_t escapeType = 0u;
         uint8_t numDigits = 0u;
         uint32_t value = 0u;
         while(next < end)
         {
            uint8_t c = *next++;
            if (isEscapeSequence)
            {
               if (escapeType == 'u')
               {
                  if (numDigits<4)
                  {
                     value<<=4;
                     value|=ASCIIHexToInt[c];
                     numDigits++;
                  }
                  if (numDigits==4)
                  {
                     //TODO: adt_str_push does not yet support unicode, will need to fix that.
                     //TODO: JSON can contain two \u sequences in a row to allow large code points. Will implement that later.
                     adt_str_push(str, (int) value);
                     escapeType = 0u;
                     numDigits = 0u;
                     value = 0u;
                  }
               }
               else
               {
                  if (c == 'u')
                  {
                     escapeType = c;
                  }
                  else
                  {
                     int32_t i;
                     for (i=0; i<NUM_ESCAPE_CHARS; i++)
                     {
                        if (c==validEscapeChars[i])
                        {
                           break;
                        }
                     }
                     if (i < NUM_ESCAPE_CHARS)
                     {
                        adt_str_push(str, escapeCharMap[i]);
                        isEscapeSequence = false;
                     }
                     else
                     {
                        bstr_set_error(ctx, BSTR_INVALID_CHARACTER_ERROR);
                        return NULL;
                     }
                  }
               }
            }
            else
            {
               if (c == quotationMark)
               {
                  return next;
               }
               else if (c == backslash)
               {
                  isEscapeSequence = true;
               }
               else if (bstr_pred_is_control_char(c))
               {
                  bstr_set_error(ctx, BSTR_INVALID_CHARACTER_ERROR);
                  return NULL;
               }
               else
               {
                  adt_error_t result = adt_str_push(str, c);
                  if (result != ADT_NO_ERROR)
                  {
                     bstr_set_error(ctx, BSTR_MEM_ERROR);
                     return NULL;
                  }
               }
            }
         }
      }
   }
   return begin;
#undef NUM_ESCAPE_CHARS
}

/**
 * searches for next line ending '\n'. returns where it encountered the line ending
 */
const uint8_t *bstr_line(const uint8_t *begin, const uint8_t *end)
{
   return bstr_search_val(begin, end, (uint8_t) '\n');
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

/**
 * Strips any whitespace from beginning of string, returns a new begin where first non-whitespace charactes is found
 */
const uint8_t *bstr_lstrip(const uint8_t *begin, const uint8_t *end)
{
   return bstr_skip_forward_while(begin, end, bstr_pred_is_whitespace);
}

/**
 * Strips any whitespace from end of string, returns a new end which points to the first whitespace character
 */
const uint8_t *bstr_rstrip(const uint8_t *begin, const uint8_t *end)
{
   return bstr_skip_backward_while(begin, end, bstr_pred_is_whitespace);
}

void bstr_strip(const uint8_t *begin, const uint8_t *end, const uint8_t **stripped_begin, const uint8_t **stripped_end)
{
   *stripped_begin = bstr_lstrip(begin, end);
   *stripped_end = bstr_rstrip(*stripped_begin, end);
}

bstr_error_t bstr_get_last_error(bstr_context_t *ctx)
{
   return ctx->last_error;
}

void bstr_clear_error(bstr_context_t *ctx)
{
   ctx->last_error = BSTR_NO_ERROR;
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

int bstr_pred_is_one_nine(int c)
{
   return (c >= '1') && (c <= '9');
}

int bstr_pred_is_control_char(int c)
{
   return (c < 32);
}

int bstr_pred_is_not_zero(int c)
{
   return c != 0u;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
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
      else if (bstr_pred_is_one_nine(c))
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

