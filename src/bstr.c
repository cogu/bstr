/*****************************************************************************
* \file      bstr.c
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
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <string.h>
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

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void bstr_set_error(bstr_context_t *ctx, bstr_error_t errorCode);
static const uint8_t *bstr_parse_number_int(bstr_context_t *ctx, const uint8_t *pBegin, const uint8_t *pEnd, bstr_number_t *number);

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
   if (self != 0)
   {
      self->lastError = BSTR_NO_ERROR;
   }
}

bstr_context_t *bstr_context_new(void)
{
   bstr_context_t *self = (bstr_context_t*) malloc(sizeof(bstr_context_t));
   if (self != 0)
   {
      bstr_context_create(self);
   }
   return self;
}

void bstr_context_delete(bstr_context_t *self)
{
   if (self != 0)
   {
      free(self);
   }
}


/**
 * Similar to strdup but operates on a bounded string. Returns a new NULL-terminated C string.
 * Additionally the caller is responsible for freeing up the memory allocated by this function
 * by calling free on the returned pointer.
 */
char* bstr_make_cstr(const uint8_t *pBegin, const uint8_t *pEnd){
   if( (pBegin != 0) && (pEnd != 0) && (pBegin<pEnd)){
      uint32_t len = (uint32_t) (pEnd-pBegin);
      uint8_t *str = (uint8_t*) malloc(len+1);
      if(str != 0){
         memcpy(str,pBegin,len);
         str[len]=0;
      }
      return (char*) str;
   }
   return 0;
}

/**
 * Similar to bstr_make but in addition it adds optional space before and after the copied string.
 * startOffset is the number of extra bytes to add before the (copied) string while
 * endOffset is the number of extra bytes to add after string.
 * It's OK to set one of the offsets to zero. If both beginOffset and endOffset are zero
 * it behaves identical to calling bstr_make_cstr directly
 */
uint8_t *bstr_make_cstr_x(const uint8_t *pBegin, const uint8_t *pEnd, uint16_t beginOffset, uint16_t endOffset){
   if( (pBegin != 0) && (pEnd != 0) && (pBegin)){
      uint8_t *str;
      uint32_t allocLen;
      uint32_t strLen = (uint32_t) (pEnd-pBegin);
      allocLen = strLen+beginOffset+endOffset+1;
      str = (uint8_t*) malloc(allocLen);
      if(str != 0){
         memcpy(str+beginOffset,pBegin,strLen);
         str[allocLen-1]=(uint8_t)0;
      }
      return str;
   }
   return 0;
}

/**
 * scans for \par val between \par pBegin and \par pEnd.
 * On success it returns the pointer to \par val.
 * On failure it returns \par pBegin if not found or NULL if invalid arguments was given.
 */
const uint8_t *bstr_search_val(const uint8_t *pBegin, const uint8_t *pEnd, uint8_t val){
   const uint8_t *pNext = pBegin;
   if (pNext > pEnd)
   {
      return 0; //invalid arguments
   }
   while(pNext < pEnd){
      uint8_t c = *pNext;
      if(c == val){
         return pNext;
      }
      pNext++;
   }
   return pBegin; //val was not found before pEnd was reached
}

/**
 * scans for matching \par left and \par right characters in a string. Used for matching '(' with ')', '[' with, ']' etc.
 * On Success it returns the pointer to \par right.
 * On failure it returns \par pBegin if the scan reached \par pEnd before \par right was found.
 * If it cannot even match \par left on the first character of \par pBegin it returns NULL.
 */
const uint8_t *bstr_match_pair(const uint8_t *pBegin, const uint8_t *pEnd, uint8_t left, uint8_t right, uint8_t escapeChar){
   const uint8_t *pNext = pBegin;
   uint32_t innerLevelCount=0;
   if (pNext < pEnd){
      if (*pNext == left){
         pNext++;
         if (escapeChar != 0){
            uint8_t isEscape = 0;
            while (pNext < pEnd){
               uint8_t c = *pNext;
               if (isEscape != 0){
                  //ignore this char
                  pNext++;
                  isEscape = 0;
                  continue;
               }
               else {
                  if ( c == escapeChar ){
                     isEscape = 1;
                  }
                  else if (c == right){
                     if (innerLevelCount == 0) {
                        return pNext;
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
               pNext++;
            }
         }
         else{
            while (pNext < pEnd) {
               uint8_t c = *pNext;
               if (c == right){
                  if (innerLevelCount == 0) {
                     return pNext;
                  }
                  else {
                     innerLevelCount--;
                  }
               }
               else if (c == left)
               {
                  innerLevelCount++;
               }
               pNext++;
            }
         }
      }
      else
      {
         return 0; //string does not start with \par left character
      }
   }
   return pBegin;
}

/**
 * \brief compares characters in string bounded by pStrBegin and pStrEnd in buffer bound by pBegin and pEnd
 * \param pBegin start of buffer
 * \param pEnd end of buffer
 * \param pStrBegin start of string to be matched
 * \param pStrEnd end of string to matched
 * \return On succes, pointer in buffer where the match stopped. On match failure it returns 0. If pEnd was reached before pStr was fully matched it returns pBegin.
 */
const uint8_t *bstr_match_bstr(const uint8_t *pBegin, const uint8_t *pEnd, const uint8_t *pStrBegin, const uint8_t *pStrEnd)
{
   const uint8_t *pNext = pBegin;
   const uint8_t *pStrNext = pStrBegin;
   if ( (pBegin > pEnd) || (pStrBegin > pStrEnd) )
   {
      errno = EINVAL; //invalid arguments
      return 0;
   }
   while(pNext < pEnd){
      if (pStrNext < pStrEnd)
      {
         if (*pNext != *pStrNext)
         {
            return 0; //string did not match
         }
      }
      else
      {
         //All characters in pStr has been successfully matched
         return pNext; //pNext should point to pStrEnd at this point
      }
      pNext++;
      pStrNext++;
   }
   if (pStrNext == pStrEnd)
   {
      return pNext; //All characters in pStr has been successfully matched
   }
   return pBegin; //reached pEnd before pStr was fully matched
}

/**
 * Checks if the C string (cstr) is a substring of the bounded string (bstr).
 */
const uint8_t *bstr_match_cstr(const uint8_t *pBegin, const uint8_t *pEnd, const char *cstr)
{
   const uint8_t *pStrBegin = (const uint8_t*) cstr;
   const uint8_t *pStrEnd;
   if ( (pBegin == 0) || (pEnd == 0) || (pEnd < pBegin) || (cstr == 0) )
   {
      errno = EINVAL; //invalid arguments
      return 0;
   }
   pStrEnd = pStrBegin + strlen(cstr);
   return bstr_match_bstr(pBegin, pEnd, pStrBegin, pStrEnd);
}

const uint8_t *bstr_to_long(const uint8_t *pBegin, const uint8_t *pEnd, long *data)
{
   const uint8_t *pResult = bstr_while_predicate(pBegin, pEnd, bstr_pred_is_digit);
   if (pResult > pBegin)
   {
      int radix=1;
      int i;
      int len = (int) (pResult-pBegin);
      if (len == 0)
      {
         return pBegin;
      }
      for (i=1;i<len;i++)
      {
         radix*=10;
      }
      *data=0;
      for(i=0;i<len;i++)
      {
         *data+=(pBegin[i]-0x30)*radix;
         radix/=10;
      }
   }
   return pResult;
}

const uint8_t *bstr_to_unsigned_long(const uint8_t *pBegin, const uint8_t *pEnd, uint8_t base, unsigned long *data)
{
   const uint8_t *pResult;
   int (*pred_func)(int c) = 0;
   if ( (base == 0) || (base == 10) )
   {
      pred_func = bstr_pred_is_digit;
      base = 10;
   }
   else if (base == 16)
   {
      pred_func = bstr_pred_is_hex_digit;
   }
   else
   {
      errno = EINVAL; //unsupported base
      return 0;
   }
   pResult = bstr_while_predicate(pBegin, pEnd, pred_func);
   if (pResult > pBegin)
   {
      unsigned long radix=1;
      int i;
      int len= (int) (pResult-pBegin);
      unsigned long tmp = 0;
      if (len == 0)
      {
         return pBegin;
      }
      for (i=1;i<len;i++)
      {
         radix*=base;
      }

      if (base == 16)
      {
         for(i=0;i<len;i++)
         {
            int tmp2 = ASCIIHexToInt[pBegin[i]];
            assert(tmp2 >= 0);
            tmp += tmp2*radix;
            radix/=base;
         }
      }
      else if ( (base == 8) || (base == 10) )
      {
         for(i=0;i<len;i++)
         {
            tmp += (pBegin[i]-0x30)*radix;
            radix/=base;
         }
      }
      else
      {
         assert(0);
      }
      *data = tmp;
   }
   return pResult;
}

/**
 * Parses a number from a bounded string using JSON number format
 */
const uint8_t *bstr_parse_json_number(bstr_context_t *ctx, const uint8_t *pBegin, const uint8_t *pEnd, bstr_number_t *number)
{
   const uint8_t *pResult;
   const uint8_t *pNext = pBegin;
   if ( (ctx == 0) || (pBegin == 0) || (pEnd == 0) || (number == 0) || (pBegin > pEnd) )
   {
      errno = EINVAL; //invalid arguments
      return 0;
   }
   number->hasInteger = false;
   number->hasFraction = false;
   number->hasExponent = false;
   number->isNegative = false;
   if (pNext < pEnd)
   {
      pResult = bstr_parse_number_int(ctx, pNext, pEnd, number);
      pNext = pResult;
   }
   else
   {
      //empty string
   }
   return pNext;
}

/**
 * Using the JSON definition, this function parses a double-quoted string literal.
 * The parsed string (not including the the quotation marks) will be stored in the str parameter
 */
const uint8_t *bstr_parse_json_string_literal(bstr_context_t *ctx, const uint8_t *pBegin, const uint8_t *pEnd, adt_str_t *str)
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

   if ( (ctx == 0) || (pBegin == 0) || (pEnd == 0) || (str == 0) || (pEnd < pBegin) )
   {
      errno = EINVAL;
      return (const uint8_t*) 0;
   }
   if (pBegin < pEnd)
   {
      uint8_t firstChar = *pBegin;
      if (firstChar == quotationMark)
      {
         const uint8_t *pNext = pBegin+1;
         bool isEscapeSequence = false;
         uint8_t escapeType = 0u;
         uint8_t numDigits = 0u;
         uint32_t value = 0u;
         while(pNext < pEnd)
         {
            uint8_t c = *pNext++;
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
                        return (const uint8_t*) 0;
                     }
                  }
               }
            }
            else
            {
               if (c == quotationMark)
               {
                  return pNext;
               }
               else if (c == backslash)
               {
                  isEscapeSequence = true;
               }
               else if (bstr_pred_is_control_char(c))
               {
                  bstr_set_error(ctx, BSTR_INVALID_CHARACTER_ERROR);
                  return (const uint8_t*) 0;
               }
               else
               {
                  adt_error_t result = adt_str_push(str, c);
                  if (result != ADT_NO_ERROR)
                  {
                     bstr_set_error(ctx, BSTR_MEM_ERROR);
                     return (const uint8_t*) 0;
                  }
               }
            }
         }
      }
   }
   return pBegin;
#undef NUM_ESCAPE_CHARS
}

/**
 * searches for next line ending '\n'. returns where it encountered the line ending
 */
const uint8_t *bstr_line(const uint8_t *pBegin, const uint8_t *pEnd)
{
   return bstr_search_val(pBegin, pEnd, (uint8_t) '\n');
}

const uint8_t *bstr_while_predicate(const uint8_t *pBegin, const uint8_t *pEnd, int (*pred_func)(int c) )
{
   const uint8_t *pNext = pBegin;
   while (pNext < pEnd)
   {
      int c = (int) *pNext;
      if (!pred_func(c)){
         break;
      }
      pNext++;
   }
   return pNext;
}

const uint8_t *bstr_while_predicate_reverse(const uint8_t *pBegin, const uint8_t *pEnd, int (*pred_func)(int c) )
{
   if (pBegin < pEnd)
   {
      const uint8_t *pNext = pEnd;
      while (pNext > pBegin)
      {
         const uint8_t *pTest = pNext-1;
         int c = (int) *pTest;
         if (!pred_func(c)){
            break;
         }
         pNext--;
      }
      return pNext;
   }
   return pBegin;
}

/**
 * Strips any whitespace from beginning of string, returns a new pBegin where first non-whitespace charactes is found
 */
const uint8_t *bstr_lstrip(const uint8_t *pBegin, const uint8_t *pEnd)
{
   return bstr_while_predicate(pBegin, pEnd, bstr_pred_is_whitespace);
}

/**
 * Strips any whitespace from end of string, returns a new pEnd which points to the first whitespace character
 */
const uint8_t *bstr_rstrip(const uint8_t *pBegin, const uint8_t *pEnd)
{
   return bstr_while_predicate_reverse(pBegin, pEnd, bstr_pred_is_whitespace);
}

void bstr_strip(const uint8_t *pBegin, const uint8_t *pEnd, const uint8_t **strippedBegin, const uint8_t **strippedEnd)
{
   *strippedBegin = bstr_lstrip(pBegin, pEnd);
   *strippedEnd = bstr_rstrip(*strippedBegin, pEnd);
}

bstr_error_t bstr_get_last_error(bstr_context_t *ctx)
{
   return ctx->lastError;
}

void bstr_clear_error(bstr_context_t *ctx)
{
   ctx->lastError = BSTR_NO_ERROR;
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

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void bstr_set_error(bstr_context_t *ctx, bstr_error_t errorCode)
{
   ctx->lastError = errorCode;
}

const uint8_t *bstr_parse_number_int(bstr_context_t *ctx, const uint8_t *pBegin, const uint8_t *pEnd, bstr_number_t *number)
{
   const uint8_t *pNext = pBegin;
   const int32_t base = 10;

   if (pNext < pEnd)
   {
      char c =  (char) *pNext;
      if (c == '-')
      {
         number->isNegative = true;
         ++pNext;
         if (pNext < pEnd)
         {
            c =  (char) *pNext;
         }
         else
         {
            bstr_set_error(ctx, BSTR_PARSE_ERROR);
            return (const uint8_t *) 0;
         }
      }
      if (c == '0')
      {
         number->integer = 0;
         number->hasInteger = true;
         ++pNext;
      }
      else if (bstr_pred_is_one_nine(c))
      {
         int64_t intPart = ASCIIHexToInt[(int) c];
         pNext++;
         while(pNext < pEnd)
         {
            int tmp =  (int) *pNext;
            if (bstr_pred_is_digit(tmp))
            {
               intPart *= base;
               intPart += ASCIIHexToInt[tmp];
               if (intPart > UINT32_MAX)
               {
                  bstr_set_error(ctx, BSTR_NUMBER_TOO_LARGE_ERROR);
                  return (const uint8_t *) 0;
               }
               pNext++;
            }
            else
            {
               break; //possible start of fraction
            }
         }
         number->hasInteger = true;
         number->integer = (uint32_t) intPart;
      }
      else
      {
         bstr_set_error(ctx, BSTR_PARSE_ERROR);
         pNext = 0;
      }
   }
   return pNext;
}

