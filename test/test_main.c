/*****************************************************************************
* \file      test_main.c
* \author    Conny Gustafsson
* \date      2017-08-04
* \brief     Unit test entry point
*
* Copyright (c) 2017-2026 Conny Gustafsson
* SPDX-License-Identifier: MIT
* See LICENSE in project root for full license terms.
******************************************************************************/
#include <stdio.h>
#include "CuTest.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


CuSuite* testsuite_bstr(void);


void streambuf_lock(void){}
void streambuf_unlock(void){}

void RunAllTests(void)
{
   CuString *output = CuStringNew();
   CuSuite* suite = CuSuiteNew();

   CuSuiteAddSuite(suite, testsuite_bstr());

   CuSuiteRun(suite);
   CuSuiteSummary(suite, output);
   CuSuiteDetails(suite, output);
   printf("%s\n", output->buffer);
   CuSuiteDelete(suite);
   CuStringDelete(output);

}

int main(void)
{
   RunAllTests();
   return 0;
}
