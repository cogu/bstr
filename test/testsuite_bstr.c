

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
static void test_bstr_while_predicate_reverse(CuTest* tc);
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

   SUITE_ADD_TEST(suite, test_bstr_while_predicate_reverse);
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

static void test_bstr_while_predicate_reverse(CuTest* tc)
{
   const char *test1 = "";
   const char *test2 = "a";
   const char *test3 = "aa";
   const char *test4 = "aa\t";
   const char *test5 = "hello        ";
   const char *test;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult;

   test = test1;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_while_predicate_reverse(pBegin, pEnd, bstr_pred_is_horizontal_space);
   CuAssertConstPtrEquals(tc, pEnd, pResult);

   test = test2;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_while_predicate_reverse(pBegin, pEnd, bstr_pred_is_horizontal_space);
   CuAssertConstPtrEquals(tc, pEnd, pResult);

   test = test3;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_while_predicate_reverse(pBegin, pEnd, bstr_pred_is_horizontal_space);
   CuAssertConstPtrEquals(tc, pEnd, pResult);

   test = test4;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_while_predicate_reverse(pBegin, pEnd, bstr_pred_is_horizontal_space);
   CuAssertConstPtrEquals(tc, pEnd-1, pResult);

   test = test5;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_while_predicate_reverse(pBegin, pEnd, bstr_pred_is_horizontal_space);
   CuAssertConstPtrEquals(tc, pEnd-8, pResult);

}

static void test_bstr_to_unsigned_long_base10(CuTest* tc)
{
   const char *test_data1 = "123456789";
   const char *test_data2 = "0";
   const char *test_data3 = "4294967295";
   const char *test_data = 0;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult = 0;
   unsigned long value;


   test_data = test_data1;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_unsigned_long(pBegin, pEnd, 10, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertUIntEquals(tc, 123456789, value);

   test_data = test_data2;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_unsigned_long(pBegin, pEnd, 10, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertUIntEquals(tc, 0, value);

   test_data = test_data3;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_unsigned_long(pBegin, pEnd, 10, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertUIntEquals(tc, 4294967295UL, value);

}

static void test_bstr_to_unsigned_long_base16(CuTest* tc)
{
   const char *test_data1 = "75BCD15";
   const char *test_data2 = "0";
   const char *test_data3 = "FFFFFFFF";
   const char *test_data = 0;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult = 0;
   unsigned long value;


   test_data = test_data1;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_unsigned_long(pBegin, pEnd, 16, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertUIntEquals(tc, 123456789, value);

   test_data = test_data2;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_unsigned_long(pBegin, pEnd, 16, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertUIntEquals(tc, 0, value);

   test_data = test_data3;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_unsigned_long(pBegin, pEnd, 16, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertUIntEquals(tc, 4294967295UL, value);

}
static void test_bstr_parse_json_number_empty(CuTest* tc)
{
   bstr_context_t ctx;
   bstr_number_t number;
   const char *test_data_empty = "";
   const char *test_data = 0;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult = 0;

   bstr_context_create(&ctx);
   test_data = test_data_empty;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
   CuAssertConstPtrEquals(tc, pResult, pEnd);

}

static void test_bstr_parse_json_number_zero(CuTest* tc)
{
   bstr_context_t ctx;
   bstr_number_t number;
   const char *test_data1 = "0";
   const char *test_data = 0;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult = 0;

   bstr_context_create(&ctx);
   test_data = test_data1;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertTrue(tc, number.hasInteger);
   CuAssertIntEquals(tc, 0, number.integer);
}

static void test_bstr_parse_json_number_single_digit_int(CuTest* tc)
{
   bstr_context_t ctx;
   char test_data[2] = {0, 0};
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult = 0;
   int i;

   bstr_context_create(&ctx);
   pBegin = (const uint8_t*) &test_data[0], pEnd = (const uint8_t*) &test_data[1];

   for (i=0;i<=9;i++)
   {
      char msg[32];
      bstr_number_t number;
      sprintf(msg, "i=%d", i);
      test_data[0] = '0' + i;
      pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
      CuAssertConstPtrEquals_Msg(tc, msg, pEnd, pResult);
      CuAssert(tc, msg, number.hasInteger);
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
   const char *test_data = 0;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult = 0;

   bstr_context_create(&ctx);
   test_data = test_data1;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertTrue(tc, number.hasInteger);
   CuAssertTrue(tc, !number.isNegative);
   CuAssertUIntEquals(tc, 100u, number.integer);

   test_data = test_data2;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertTrue(tc, number.hasInteger);
   CuAssertTrue(tc, !number.isNegative);
   CuAssertUIntEquals(tc, 12345u, number.integer);

   test_data = test_data3;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertTrue(tc, number.hasInteger);
   CuAssertTrue(tc, !number.isNegative);
   CuAssertUIntEquals(tc, 999999999u, number.integer);

   bstr_clear_error(&ctx);
   test_data = test_data4;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
   CuAssertConstPtrEquals(tc, NULL, pResult);
   CuAssertUIntEquals(tc, BSTR_NUMBER_TOO_LARGE_ERROR, bstr_get_last_error(&ctx));

   test_data = test_data5;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertTrue(tc, number.hasInteger);
   CuAssertTrue(tc, !number.isNegative);
   CuAssertUIntEquals(tc, 2147483648u, number.integer);

   bstr_clear_error(&ctx);
   test_data = test_data6;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertTrue(tc, number.hasInteger);
   CuAssertTrue(tc, !number.isNegative);
   CuAssertUIntEquals(tc, INT32_MAX, number.integer);

}

static void test_bstr_parse_json_number_negative_int(CuTest* tc)
{
   bstr_context_t ctx;
   bstr_number_t number;
   const char *test_data1 = "-1";
   const char *test_data2 = "-200";
   const char *test_data = 0;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult = 0;

   bstr_context_create(&ctx);

   test_data = test_data1;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertTrue(tc, number.hasInteger);
   CuAssertTrue(tc, number.isNegative);
   CuAssertUIntEquals(tc, 1u, number.integer);

   test_data = test_data2;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_parse_json_number(&ctx, pBegin, pEnd, &number);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertTrue(tc, number.hasInteger);
   CuAssertTrue(tc, number.isNegative);
   CuAssertUIntEquals(tc, 200u, number.integer);

}



static void test_bstr_lstrip(CuTest* tc)
{
   const char *test1 = " ";
   const char *test2 = "   5";
   const char *test3 = "\t5";
   const char *test;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult;

   test = test1;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_lstrip(pBegin, pEnd);
   CuAssertConstPtrEquals(tc, pBegin+1, pResult);

   test = test2;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_lstrip(pBegin, pEnd);
   CuAssertConstPtrEquals(tc, pBegin+3, pResult);

   test = test3;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_lstrip(pBegin, pEnd);
   CuAssertConstPtrEquals(tc, pBegin+1, pResult);

}

static void test_bstr_rstrip(CuTest* tc)
{
   const char *test1 = " ";
   const char *test2 = ", ";
   const char *test3 = "33   ";
   const char *test;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult;

   test = test1;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_rstrip(pBegin, pEnd);
   CuAssertConstPtrEquals(tc, pEnd-1, pResult);

   test = test2;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_rstrip(pBegin, pEnd);
   CuAssertConstPtrEquals(tc, pEnd-1, pResult);

   test = test3;
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_rstrip(pBegin, pEnd);
   CuAssertConstPtrEquals(tc, pEnd-3, pResult);
}

static void test_bstr_parse_json_string_literal_empty(CuTest* tc)
{
   const char *test = "\"\"";
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult;
   adt_str_t *str;
   bstr_context_t ctx;

   str = adt_str_new_utf8();
   bstr_context_create(&ctx);
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_parse_json_string_literal(&ctx, pBegin, pEnd, str);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertIntEquals(tc, 0, adt_str_length(str));

   adt_str_delete(str);
}

static void test_bstr_parse_json_string_literal_ascii(CuTest* tc)
{
   const char *test1 = "\"Test1\"";
   const char *test2 = "\"Hello World\"";
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult;
   adt_str_t *str;
   bstr_context_t ctx;
   const char *test;

   bstr_context_create(&ctx);

   test = test1;
   str = adt_str_new_utf8();
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_parse_json_string_literal(&ctx, pBegin, pEnd, str);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertStrEquals(tc, "Test1", adt_str_cstr(str));
   adt_str_delete(str);

   test = test2;
   str = adt_str_new_utf8();
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_parse_json_string_literal(&ctx, pBegin, pEnd, str);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertStrEquals(tc, "Hello World", adt_str_cstr(str));
   adt_str_delete(str);
}

static void test_bstr_parse_json_string_literal_escapeChars(CuTest* tc)
{
   const char *test1 = "\"Hello\\r\\nWorld\\f\"";
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult;
   adt_str_t *str;
   bstr_context_t ctx;
   const char *test;

   bstr_context_create(&ctx);

   test = test1;
   str = adt_str_new_utf8();
   pBegin = (const uint8_t*) test, pEnd = (const uint8_t*) (test + strlen(test));
   pResult = bstr_parse_json_string_literal(&ctx, pBegin, pEnd, str);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
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
   const char *test_data = 0;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   const uint8_t *pResult = 0;
   double value;
   const double delta = 0.0001;

   test_data = test_data1;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_double(pBegin, pEnd, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertDblEquals(tc, 0.0, value, delta);

   test_data = test_data2;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_double(pBegin, pEnd, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertDblEquals(tc, 0.0, value, delta);

   test_data = test_data3;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_double(pBegin, pEnd, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertDblEquals(tc, 1.0, value, delta);

   test_data = test_data4;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_double(pBegin, pEnd, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertDblEquals(tc, 0.1, value, delta);

   test_data = test_data5;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_double(pBegin, pEnd, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertDblEquals(tc, -1.0, value, delta);

   test_data = test_data6;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_double(pBegin, pEnd, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertDblEquals(tc, -1.0, value, delta);

   test_data = test_data7;
   pBegin = (const uint8_t*) test_data, pEnd = pBegin+strlen(test_data);
   pResult = bstr_to_double(pBegin, pEnd, &value);
   CuAssertConstPtrEquals(tc, pEnd, pResult);
   CuAssertDblEquals(tc, -100.123, value, delta);

}
