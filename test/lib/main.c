/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2024-04-28
 * Author: Aryadev Chavali
 * Description:
 */

#include "test-base.h"

#include "test-darr.h"

int main(void)
{
  RUN_TEST_SUITE(test_lib_base);
  RUN_TEST_SUITE(test_lib_darr);
  return 0;
}
