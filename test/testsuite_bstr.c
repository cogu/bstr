/*****************************************************************************
* \file      testsuite_bstr.c
* \author    Conny Gustafsson
* \date      2017-08-04
* \brief     Unit tests for bstr
*
* Copyright (c) 2017-2026 Conny Gustafsson
* SPDX-License-Identifier: MIT
* See LICENSE in project root for full license terms.
******************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "CuTest.h"
#include "bstr.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_bstr_make_cstr(CuTest* tc);
static void test_bstr_make_cstr_x(CuTest* tc);
static void test_bstr_search_val(CuTest* tc);
static void test_bstr_match_pair(CuTest* tc);
static void test_bstr_match_string(CuTest* tc);
static void test_bstr_to_long(CuTest* tc);
static void test_bstr_line_and_strip(CuTest* tc);
static void test_bstr_predicates(CuTest* tc);
static void test_bstr_parse_json_string_literal_invalid(CuTest* tc);
static void test_bstr_skip_backward_while(CuTest* tc);
static void test_bstr_to_unsigned_long_base10(CuTest* tc);
static void test_bstr_to_unsigned_long_base16(CuTest* tc);
static void test_bstr_parse_json_number_empty(CuTest* tc);
static void test_bstr_parse_json_number_zero(CuTest* tc);
static void test_bstr_parse_json_number_single_digit_int(CuTest* tc);
static void test_bstr_parse_json_number_multi_digit_int(CuTest* tc);
static void test_bstr_parse_json_number_negative_int(CuTest* tc);
static void test_bstr_lstrip(CuTest* tc);
static void test_bstr_rstrip(CuTest* tc);
static void test_bstr_parse_json_string_literal_empty(CuTest* tc);
static void test_bstr_parse_json_string_literal_ascii(CuTest* tc);
static void test_bstr_parse_json_string_literal_escapeChars(CuTest* tc);
static void test_bstr_to_double(CuTest* tc);




//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////


CuSuite* testsuite_bstr(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_bstr_make_cstr);
   SUITE_ADD_TEST(suite, test_bstr_make_cstr_x);
   SUITE_ADD_TEST(suite, test_bstr_search_val);
   SUITE_ADD_TEST(suite, test_bstr_match_pair);
   SUITE_ADD_TEST(suite, test_bstr_match_string);
   SUITE_ADD_TEST(suite, test_bstr_to_long);
   SUITE_ADD_TEST(suite, test_bstr_line_and_strip);
   SUITE_ADD_TEST(suite, test_bstr_predicates);
   SUITE_ADD_TEST(suite, test_bstr_parse_json_string_literal_invalid);
   SUITE_ADD_TEST(suite, test_bstr_skip_backward_while);
   SUITE_ADD_TEST(suite, test_bstr_to_unsigned_long_base10);
   SUITE_ADD_TEST(suite, test_bstr_to_unsigned_long_base16);
   SUITE_ADD_TEST(suite, test_bstr_parse_json_number_empty);
   SUITE_ADD_TEST(suite, test_bstr_parse_json_number_zero);
   SUITE_ADD_TEST(suite, test_bstr_parse_json_number_single_digit_int);
   SUITE_ADD_TEST(suite, test_bstr_parse_json_number_multi_digit_int);
   SUITE_ADD_TEST(suite, test_bstr_parse_json_number_negative_int);
   SUITE_ADD_TEST(suite, test_bstr_lstrip);
   SUITE_ADD_TEST(suite, test_bstr_rstrip);
   SUITE_ADD_TEST(suite, test_bstr_parse_json_string_literal_empty);
   SUITE_ADD_TEST(suite, test_bstr_parse_json_string_literal_ascii);
   SUITE_ADD_TEST(suite, test_bstr_parse_json_string_literal_escapeChars);
   SUITE_ADD_TEST(suite, test_bstr_to_double);


   return suite;
}
//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void test_bstr_make_cstr(CuTest* tc)
{
   const uint8_t data[] = {'h', 'e', 'l', 'l', 'o'};
   char *result = bstr_make_cstr(data, data + sizeof(data));

   CuAssertPtrNotNull(tc, result);
   CuAssertStrEquals(tc, "hello", result);
   free(result);

   CuAssertPtrEquals(tc, NULL, bstr_make_cstr(NULL, data + sizeof(data)));
   CuAssertPtrEquals(tc, NULL, bstr_make_cstr(data, NULL));
   CuAssertPtrEquals(tc, NULL, bstr_make_cstr(data, data));
   CuAssertPtrEquals(tc, NULL, bstr_make_cstr(data + sizeof(data), data));
}

static void test_bstr_make_cstr_x(CuTest* tc)
{
   const uint8_t data[] = {'d', 'a', 't', 'a'};
   char *result = bstr_make_cstr_x(data, data + sizeof(data), 2, 3);

   CuAssertPtrNotNull(tc, result);
   CuAssertTrue(tc, memcmp(result + 2, data, sizeof(data)) == 0);
   CuAssertIntEquals(tc, '\0', result[9]);
   free(result);

   result = bstr_make_cstr_x(data, data + sizeof(data), 0, 0);
   CuAssertPtrNotNull(tc, result);
   CuAssertStrEquals(tc, "data", result);
   free(result);

   CuAssertPtrEquals(tc, NULL, bstr_make_cstr_x(NULL, data + sizeof(data), 1, 1));
   CuAssertPtrEquals(tc, NULL, bstr_make_cstr_x(data, NULL, 1, 1));
}

static void test_bstr_search_val(CuTest* tc)
{
   const uint8_t data[] = {'a', 0, 'b', 'c', 0xff};
   const uint8_t *begin = data;
   const uint8_t *end = data + sizeof(data);

   CuAssertConstPtrEquals(tc, begin, bstr_search_val(begin, end, 'a'));
   CuAssertConstPtrEquals(tc, begin + 1, bstr_search_val(begin, end, 0));
   CuAssertConstPtrEquals(tc, begin + 4, bstr_search_val(begin, end, 0xff));
   CuAssertConstPtrEquals(tc, begin, bstr_search_val(begin, end, 'z'));
   CuAssertConstPtrEquals(tc, begin, bstr_search_val(begin, begin, 'a'));
   CuAssertConstPtrEquals(tc, NULL, bstr_search_val(end, begin, 'a'));
}

static void test_bstr_match_pair(CuTest* tc)
{
   const char simple[] = "(value)";
   const char nested[] = "((inner))";
   const char escaped[] = "(value\\)tail)";
   const char unclosed[] = "(value";
   const char no_open[] = "value)";

   CuAssertConstPtrEquals(tc, (const uint8_t*) simple + strlen(simple) - 1,
      bstr_match_pair((const uint8_t*) simple, (const uint8_t*) simple + strlen(simple), '(', ')', 0));
   CuAssertConstPtrEquals(tc, (const uint8_t*) nested + strlen(nested) - 1,
      bstr_match_pair((const uint8_t*) nested, (const uint8_t*) nested + strlen(nested), '(', ')', 0));
   CuAssertConstPtrEquals(tc, (const uint8_t*) escaped + strlen(escaped) - 1,
      bstr_match_pair((const uint8_t*) escaped, (const uint8_t*) escaped + strlen(escaped), '(', ')', '\\'));
   CuAssertConstPtrEquals(tc, (const uint8_t*) unclosed,
      bstr_match_pair((const uint8_t*) unclosed, (const uint8_t*) unclosed + strlen(unclosed), '(', ')', 0));
   CuAssertConstPtrEquals(tc, NULL,
      bstr_match_pair((const uint8_t*) no_open, (const uint8_t*) no_open + strlen(no_open), '(', ')', 0));
}

static void test_bstr_match_string(CuTest* tc)
{
   const uint8_t buffer[] = "hello";
   const uint8_t match[] = "hel";
   const uint8_t mismatch[] = "hex";
   const uint8_t too_long[] = "hello!";
   const uint8_t *begin = buffer;
   const uint8_t *end = buffer + strlen((const char*) buffer);

   CuAssertConstPtrEquals(tc, begin + 3,
      bstr_match_bstr(begin, end, match, match + strlen((const char*) match)));
   CuAssertConstPtrEquals(tc, NULL,
      bstr_match_bstr(begin, end, mismatch, mismatch + strlen((const char*) mismatch)));
   CuAssertConstPtrEquals(tc, begin,
      bstr_match_bstr(begin, end, too_long, too_long + strlen((const char*) too_long)));
   CuAssertConstPtrEquals(tc, begin + 5, bstr_match_cstr(begin, end, "hello"));
   CuAssertConstPtrEquals(tc, begin, bstr_match_cstr(begin, end, ""));
   CuAssertConstPtrEquals(tc, NULL, bstr_match_cstr(begin, end, NULL));
}

static void test_bstr_to_long(CuTest* tc)
{
   const char negative[] = "-123";
   const char hexadecimal[] = "0x1f";
   const char octal[] = "077";
   const char partial[] = "12rest";
   long value = 0;

   CuAssertConstPtrEquals(tc, (const uint8_t*) negative + strlen(negative),
      bstr_to_long((const uint8_t*) negative, (const uint8_t*) negative + strlen(negative), &value));
   CuAssertLIntEquals(tc, -123, value);

   CuAssertConstPtrEquals(tc, (const uint8_t*) hexadecimal + strlen(hexadecimal),
      bstr_to_long((const uint8_t*) hexadecimal, (const uint8_t*) hexadecimal + strlen(hexadecimal), &value));
   CuAssertLIntEquals(tc, 31, value);

   CuAssertConstPtrEquals(tc, (const uint8_t*) octal + strlen(octal),
      bstr_to_long((const uint8_t*) octal, (const uint8_t*) octal + strlen(octal), &value));
   CuAssertLIntEquals(tc, 63, value);

   CuAssertConstPtrEquals(tc, (const uint8_t*) partial + 2,
      bstr_to_long((const uint8_t*) partial, (const uint8_t*) partial + strlen(partial), &value));
   CuAssertLIntEquals(tc, 12, value);
}

static void test_bstr_line_and_strip(CuTest* tc)
{
   const char lines[] = "first\nsecond\n";
   const char no_line[] = "first";
   const char padded[] = " \tvalue\r\n";
   const char whitespace[] = " \t\r\n";
   const uint8_t *stripped_begin;
   const uint8_t *stripped_end;

   CuAssertConstPtrEquals(tc, (const uint8_t*) lines + 5,
      bstr_line((const uint8_t*) lines, (const uint8_t*) lines + strlen(lines)));
   CuAssertConstPtrEquals(tc, (const uint8_t*) no_line,
      bstr_line((const uint8_t*) no_line, (const uint8_t*) no_line + strlen(no_line)));

   bstr_strip((const uint8_t*) padded, (const uint8_t*) padded + strlen(padded),
      &stripped_begin, &stripped_end);
   CuAssertConstPtrEquals(tc, (const uint8_t*) padded + 2, stripped_begin);
   CuAssertConstPtrEquals(tc, (const uint8_t*) padded + 7, stripped_end);

   bstr_strip((const uint8_t*) whitespace, (const uint8_t*) whitespace + strlen(whitespace),
      &stripped_begin, &stripped_end);
   CuAssertConstPtrEquals(tc, (const uint8_t*) whitespace + strlen(whitespace), stripped_begin);
   CuAssertConstPtrEquals(tc, stripped_begin, stripped_end);
}

static void test_bstr_predicates(CuTest* tc)
{
   CuAssertTrue(tc, bstr_pred_is_horizontal_space(' '));
   CuAssertTrue(tc, bstr_pred_is_horizontal_space('\t'));
   CuAssertFalse(tc, bstr_pred_is_horizontal_space('\n'));
   CuAssertTrue(tc, bstr_pred_is_whitespace('\n'));
   CuAssertTrue(tc, bstr_pred_is_whitespace('\r'));
   CuAssertFalse(tc, bstr_pred_is_whitespace('x'));
   CuAssertTrue(tc, bstr_pred_is_digit('0'));
   CuAssertTrue(tc, bstr_pred_is_digit('9'));
   CuAssertFalse(tc, bstr_pred_is_digit('a'));
   CuAssertTrue(tc, bstr_pred_is_hex_digit('a'));
   CuAssertTrue(tc, bstr_pred_is_hex_digit('F'));
   CuAssertFalse(tc, bstr_pred_is_hex_digit('g'));
   CuAssertFalse(tc, bstr_pred_is_one_nine('0'));
   CuAssertTrue(tc, bstr_pred_is_one_nine('1'));
   CuAssertTrue(tc, bstr_pred_is_control_char(0x1f));
   CuAssertFalse(tc, bstr_pred_is_control_char(' '));
   CuAssertFalse(tc, bstr_pred_is_not_zero(0));
   CuAssertTrue(tc, bstr_pred_is_not_zero(1));
}

static void test_bstr_parse_json_string_literal_invalid(CuTest* tc)
{
   const char invalid_escape[] = "\"bad\\q\"";
   const char control_character[] = {'\"', 'a', 0x01, 'b', '\"', '\0'};
   const char unterminated[] = "\"value";
   bstr_context_t ctx;
   adt_str_t *str;
   const uint8_t *result;

   bstr_context_create(&ctx);
   str = adt_str_new_utf8();
   result = bstr_parse_json_string_literal(&ctx, (const uint8_t*) invalid_escape,
      (const uint8_t*) invalid_escape + strlen(invalid_escape), str);
   CuAssertConstPtrEquals(tc, NULL, result);
   CuAssertUIntEquals(tc, BSTR_INVALID_CHARACTER_ERROR, bstr_get_last_error(&ctx));
   adt_str_delete(str);

   bstr_clear_error(&ctx);
   str = adt_str_new_utf8();
   result = bstr_parse_json_string_literal(&ctx, (const uint8_t*) control_character,
      (const uint8_t*) control_character + strlen(control_character), str);
   CuAssertConstPtrEquals(tc, NULL, result);
   CuAssertUIntEquals(tc, BSTR_INVALID_CHARACTER_ERROR, bstr_get_last_error(&ctx));
   adt_str_delete(str);

   bstr_clear_error(&ctx);
   str = adt_str_new_utf8();
   result = bstr_parse_json_string_literal(&ctx, (const uint8_t*) unterminated,
      (const uint8_t*) unterminated + strlen(unterminated), str);
   CuAssertConstPtrEquals(tc, (const uint8_t*) unterminated, result);
   CuAssertUIntEquals(tc, BSTR_NO_ERROR, bstr_get_last_error(&ctx));
   adt_str_delete(str);
}

static void test_bstr_skip_backward_while(CuTest* tc)
{
   const char *test1 = "";
   const char *test2 = "a";
   const char *test3 = "aa";
   const char *test4 = "aa\t";
   const char *test5 = "hello        ";
   const char *test;
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result;

   test = test1;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_skip_backward_while(begin, end, bstr_pred_is_horizontal_space);
   CuAssertConstPtrEquals(tc, end, result);

   test = test2;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_skip_backward_while(begin, end, bstr_pred_is_horizontal_space);
   CuAssertConstPtrEquals(tc, end, result);

   test = test3;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_skip_backward_while(begin, end, bstr_pred_is_horizontal_space);
   CuAssertConstPtrEquals(tc, end, result);

   test = test4;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_skip_backward_while(begin, end, bstr_pred_is_horizontal_space);
   CuAssertConstPtrEquals(tc, end-1, result);

   test = test5;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_skip_backward_while(begin, end, bstr_pred_is_horizontal_space);
   CuAssertConstPtrEquals(tc, end-8, result);

}

static void test_bstr_to_unsigned_long_base10(CuTest* tc)
{
   const char *test_data1 = "123456789";
   const char *test_data2 = "0";
   const char *test_data3 = "4294967295";
   const char *test_data = NULL;
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result = NULL;
   unsigned long value;


   test_data = test_data1;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_unsigned_long(begin, end, 10, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertUIntEquals(tc, 123456789, value);

   test_data = test_data2;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_unsigned_long(begin, end, 10, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertUIntEquals(tc, 0, value);

   test_data = test_data3;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_unsigned_long(begin, end, 10, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertUIntEquals(tc, 4294967295UL, value);

}

static void test_bstr_to_unsigned_long_base16(CuTest* tc)
{
   const char *test_data1 = "75BCD15";
   const char *test_data2 = "0";
   const char *test_data3 = "FFFFFFFF";
   const char *test_data = NULL;
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result = NULL;
   unsigned long value;


   test_data = test_data1;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_unsigned_long(begin, end, 16, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertUIntEquals(tc, 123456789, value);

   test_data = test_data2;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_unsigned_long(begin, end, 16, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertUIntEquals(tc, 0, value);

   test_data = test_data3;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_unsigned_long(begin, end, 16, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertUIntEquals(tc, 4294967295UL, value);

}
static void test_bstr_parse_json_number_empty(CuTest* tc)
{
   bstr_context_t ctx;
   bstr_number_t number;
   const char *test_data_empty = "";
   const char *test_data = NULL;
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result = NULL;

   bstr_context_create(&ctx);
   test_data = test_data_empty;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_parse_json_number(&ctx, begin, end, &number);
   CuAssertConstPtrEquals(tc, result, end);

}

static void test_bstr_parse_json_number_zero(CuTest* tc)
{
   bstr_context_t ctx;
   bstr_number_t number;
   const char *test_data1 = "0";
   const char *test_data = NULL;
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result = NULL;

   bstr_context_create(&ctx);
   test_data = test_data1;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_parse_json_number(&ctx, begin, end, &number);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertTrue(tc, number.has_integer);
   CuAssertIntEquals(tc, 0, number.integer);
}

static void test_bstr_parse_json_number_single_digit_int(CuTest* tc)
{
   bstr_context_t ctx;
   char test_data[2] = {0, 0};
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result = NULL;
   int i;

   bstr_context_create(&ctx);
   begin = (const uint8_t*) &test_data[0], end = (const uint8_t*) &test_data[1];

   for (i=0;i<=9;i++)
   {
      char msg[32];
      bstr_number_t number;
      sprintf(msg, "i=%d", i);
      test_data[0] = '0' + i;
      result = bstr_parse_json_number(&ctx, begin, end, &number);
      CuAssertConstPtrEquals_Msg(tc, msg, end, result);
      CuAssert(tc, msg, number.has_integer);
      CuAssertIntEquals_Msg(tc,msg, i, number.integer);
   }
}

static void test_bstr_parse_json_number_multi_digit_int(CuTest* tc)
{
   bstr_context_t ctx;
   bstr_number_t number;
   const char *test_data1 = "100";
   const char *test_data2 = "12345";
   const char *test_data3 = "999999999";
   const char *test_data4 = "12345678901234567890"; //This is way outside 32-bit range
   const char *test_data5 = "2147483648"; //This is just outside 31-bit range
   const char *test_data6 = "2147483647"; //This is just inside 31-bit range
   const char *test_data = NULL;
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result = NULL;

   bstr_context_create(&ctx);
   test_data = test_data1;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_parse_json_number(&ctx, begin, end, &number);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertTrue(tc, number.has_integer);
   CuAssertTrue(tc, !number.is_negative);
   CuAssertUIntEquals(tc, 100u, number.integer);

   test_data = test_data2;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_parse_json_number(&ctx, begin, end, &number);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertTrue(tc, number.has_integer);
   CuAssertTrue(tc, !number.is_negative);
   CuAssertUIntEquals(tc, 12345u, number.integer);

   test_data = test_data3;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_parse_json_number(&ctx, begin, end, &number);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertTrue(tc, number.has_integer);
   CuAssertTrue(tc, !number.is_negative);
   CuAssertUIntEquals(tc, 999999999u, number.integer);

   bstr_clear_error(&ctx);
   test_data = test_data4;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_parse_json_number(&ctx, begin, end, &number);
   CuAssertConstPtrEquals(tc, NULL, result);
   CuAssertUIntEquals(tc, BSTR_NUMBER_TOO_LARGE_ERROR, bstr_get_last_error(&ctx));

   test_data = test_data5;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_parse_json_number(&ctx, begin, end, &number);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertTrue(tc, number.has_integer);
   CuAssertTrue(tc, !number.is_negative);
   CuAssertUIntEquals(tc, 2147483648u, number.integer);

   bstr_clear_error(&ctx);
   test_data = test_data6;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_parse_json_number(&ctx, begin, end, &number);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertTrue(tc, number.has_integer);
   CuAssertTrue(tc, !number.is_negative);
   CuAssertUIntEquals(tc, INT32_MAX, number.integer);

}

static void test_bstr_parse_json_number_negative_int(CuTest* tc)
{
   bstr_context_t ctx;
   bstr_number_t number;
   const char *test_data1 = "-1";
   const char *test_data2 = "-200";
   const char *test_data = NULL;
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result = NULL;

   bstr_context_create(&ctx);

   test_data = test_data1;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_parse_json_number(&ctx, begin, end, &number);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertTrue(tc, number.has_integer);
   CuAssertTrue(tc, number.is_negative);
   CuAssertUIntEquals(tc, 1u, number.integer);

   test_data = test_data2;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_parse_json_number(&ctx, begin, end, &number);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertTrue(tc, number.has_integer);
   CuAssertTrue(tc, number.is_negative);
   CuAssertUIntEquals(tc, 200u, number.integer);

}



static void test_bstr_lstrip(CuTest* tc)
{
   const char *test1 = " ";
   const char *test2 = "   5";
   const char *test3 = "\t5";
   const char *test;
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result;

   test = test1;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_lstrip(begin, end);
   CuAssertConstPtrEquals(tc, begin+1, result);

   test = test2;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_lstrip(begin, end);
   CuAssertConstPtrEquals(tc, begin+3, result);

   test = test3;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_lstrip(begin, end);
   CuAssertConstPtrEquals(tc, begin+1, result);

}

static void test_bstr_rstrip(CuTest* tc)
{
   const char *test1 = " ";
   const char *test2 = ", ";
   const char *test3 = "33   ";
   const char *test;
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result;

   test = test1;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_rstrip(begin, end);
   CuAssertConstPtrEquals(tc, end-1, result);

   test = test2;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_rstrip(begin, end);
   CuAssertConstPtrEquals(tc, end-1, result);

   test = test3;
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_rstrip(begin, end);
   CuAssertConstPtrEquals(tc, end-3, result);
}

static void test_bstr_parse_json_string_literal_empty(CuTest* tc)
{
   const char *test = "\"\"";
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result;
   adt_str_t *str;
   bstr_context_t ctx;

   str = adt_str_new_utf8();
   bstr_context_create(&ctx);
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_parse_json_string_literal(&ctx, begin, end, str);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertIntEquals(tc, 0, adt_str_length(str));

   adt_str_delete(str);
}

static void test_bstr_parse_json_string_literal_ascii(CuTest* tc)
{
   const char *test1 = "\"Test1\"";
   const char *test2 = "\"Hello World\"";
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result;
   adt_str_t *str;
   bstr_context_t ctx;
   const char *test;

   bstr_context_create(&ctx);

   test = test1;
   str = adt_str_new_utf8();
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_parse_json_string_literal(&ctx, begin, end, str);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertStrEquals(tc, "Test1", adt_str_cstr(str));
   adt_str_delete(str);

   test = test2;
   str = adt_str_new_utf8();
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_parse_json_string_literal(&ctx, begin, end, str);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertStrEquals(tc, "Hello World", adt_str_cstr(str));
   adt_str_delete(str);
}

static void test_bstr_parse_json_string_literal_escapeChars(CuTest* tc)
{
   const char *test1 = "\"Hello\\r\\nWorld\\f\"";
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result;
   adt_str_t *str;
   bstr_context_t ctx;
   const char *test;

   bstr_context_create(&ctx);

   test = test1;
   str = adt_str_new_utf8();
   begin = (const uint8_t*) test, end = (const uint8_t*) (test + strlen(test));
   result = bstr_parse_json_string_literal(&ctx, begin, end, str);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertStrEquals(tc, "Hello\r\nWorld\f", adt_str_cstr(str));
   adt_str_delete(str);
}

static void test_bstr_to_double(CuTest* tc)
{
   const char *test_data1 = "0";
   const char *test_data2 = "0.0";
   const char *test_data3 = "1.0";
   const char *test_data4 = "0.1";
   const char *test_data5 = "-1";
   const char *test_data6 = "-1.0";
   const char *test_data7 = "-100.123";
   const char *test_data = NULL;
   const uint8_t *begin;
   const uint8_t *end;
   const uint8_t *result = NULL;
   double value;
   const double delta = 0.0001;

   test_data = test_data1;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_double(begin, end, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertDblEquals(tc, 0.0, value, delta);

   test_data = test_data2;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_double(begin, end, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertDblEquals(tc, 0.0, value, delta);

   test_data = test_data3;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_double(begin, end, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertDblEquals(tc, 1.0, value, delta);

   test_data = test_data4;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_double(begin, end, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertDblEquals(tc, 0.1, value, delta);

   test_data = test_data5;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_double(begin, end, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertDblEquals(tc, -1.0, value, delta);

   test_data = test_data6;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_double(begin, end, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertDblEquals(tc, -1.0, value, delta);

   test_data = test_data7;
   begin = (const uint8_t*) test_data, end = begin+strlen(test_data);
   result = bstr_to_double(begin, end, &value);
   CuAssertConstPtrEquals(tc, end, result);
   CuAssertDblEquals(tc, -100.123, value, delta);

}
