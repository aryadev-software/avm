/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2024-04-28
 * Author: Aryadev Chavali
 * Description: Tests for darr.h
 */

#ifndef TEST_DARR_H
#define TEST_DARR_H

#include <lib/darr.h>

#include "../testing.h"

void test_lib_darr_init(void)
{
  struct TestCase
  {
    size_t n, expected_used, expected_available;
  } tests[] = {
      {0, 0, DARR_DEFAULT_SIZE},
      {10, 0, 10},
      {2 << 20, 0, 2 << 20},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    darr_t darr = {0};
#if VERBOSE > 1
    INFO(__func__, "Testing %lu -> (%lu, %lu)\n", tests[i].n,
         tests[i].expected_used, tests[i].expected_available);
#endif

    darr_init(&darr, tests[i].n);

    if (darr.used != tests[i].expected_used ||
        darr.available != tests[i].expected_available || !darr.data)
    {
      FAIL(__func__, "[%lu] -> Expected (%lu, %lu) got (%lu, %lu)\n", i,
           tests[i].expected_used, tests[i].expected_available, darr.used,
           darr.available);
      assert(false);
    }
    free(darr.data);
  }
}

void test_lib_darr_ensure_capacity_expands(void)
{
  struct TestCase
  {
    size_t used, available, requested_capacity, expected_available;
  } tests[] = {
      // At used,available=1 when requesting one extra we tightly fit
      {1, 1, 1, 2},
      // No extraneous allocations when we have enough capacity
      {0, 10, 1, 10},
      {7, 10, 2, 10},
      {1 << 9, 1 << 10, 1, 1 << 10},
      // When we reallocate we reallocate as much as needed
      {10, 10, 1, 10 * DARR_REALLOC_MULT},
      {10, 10, 10, 20},
      {50, 100, 300, 350},
      {1 << 20, 2 << 20, 2 << 20, 3 << 20},
      // When we reallocate we allocate MORE than needed (for
      // amortized constant)
      {1, 5, 5, 10},
      {85, 100, 40, 200},
      {4 << 20, 5 << 20, 1 << 23, 5 << 21},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    darr_t darr = {0};
#if VERBOSE > 1
    INFO(__func__, "Testing (%lu, %lu, %lu) -> %lu\n", tests[i].used,
         tests[i].available, tests[i].requested_capacity,
         tests[i].expected_available);
#endif
    darr_init(&darr, tests[i].available);
    darr.used = tests[i].used;
    darr_ensure_capacity(&darr, tests[i].requested_capacity);

    if (darr.available < tests[i].expected_available || !darr.data)
    {
      FAIL(__func__, "[%lu] -> Expected %lu got %lu\n", i,
           tests[i].expected_available, darr.available);
      assert(false);
    }

    free(darr.data);
  }
}

TEST_SUITE(test_lib_darr, CREATE_TEST(test_lib_darr_init),
           CREATE_TEST(test_lib_darr_ensure_capacity_expands), );

#endif
