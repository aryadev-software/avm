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

bool all(byte_t *arr, size_t size, byte_t b)
{
  for (size_t i = 0; i < size; ++i)
    if (arr[i] != b)
      return false;
  return true;
}

void test_lib_darr_ensure_capacity_prev_data(void)
{
  struct TestCase
  {
    size_t used, available, requested;
    char fill;
  } tests[] = {
      {100, 100, 1, 0},
      {285, 300, 200, '\n'},
      {1 << 20, 1 << 21, 2 << 20, 'a'},
  };
  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    darr_t darr = {0};
#if VERBOSE > 1
    INFO(__func__, "Testing (%lu, %lu, %lu, %d)\n", tests[i].used,
         tests[i].available, tests[i].requested, tests[i].fill);
#endif
    darr_init(&darr, tests[i].available);
    darr.used = tests[i].used;
    memset(darr.data, tests[i].fill, darr.used);
    darr_ensure_capacity(&darr, tests[i].requested);
    if (!all(darr.data, darr.used, tests[i].fill))
    {
      FAIL(__func__, "[%lu] -> Previous array data was corrupted!", i);
      assert(false);
    }
    free(darr.data);
  }
}

void test_lib_darr_append_byte(void)
{
  struct TestCase
  {
    size_t used, available;
    byte_t byte;
  } tests[] = {
      {0, 1, 'a'},           {0, 100, 'a'},           {1 << 10, 1 << 11, 'a'},
      {1 << 8, 1 << 8, 'a'}, {1 << 20, 1 << 20, 'a'},
  };
  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    darr_t darr = {0};
#if VERBOSE > 1
    INFO(__func__, "Testing (%lu, %lu) -> darr[%lu] = '%c'\n", used, available,
         used, byte);
#endif
    darr_init(&darr, tests[i].available);
    darr.used = tests[i].used;
    darr_append_byte(&darr, tests[i].byte);
    if (darr.data[tests[i].used] != tests[i].byte)
    {
      FAIL(__func__, "Expected '%c' got '%c'\n", tests[i].byte,
           darr.data[tests[i].used]);
      assert(false);
    }
    free(darr.data);
  }
}

void test_lib_darr_append_bytes(void)
{
  struct TestCase
  {
    size_t used, available, n;
    byte_t data[1024];
  } tests[] = {
      {0, 0, 4, {0}},
      {8, 10, 3, {0}},
      {1 << 20, 1 << 20, 1 << 10, {0}},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    memset(tests[i].data, 0xFF, tests[i].n);

    darr_t darr = {0};
    darr_init(&darr, tests[i].available);
    darr.used = tests[i].used;
    darr_append_bytes(&darr, tests[i].data, tests[i].n);

    if (darr.available < tests[i].used + tests[i].n ||
        memcmp(darr.data + tests[i].used, tests[i].data, tests[i].n) != 0)
    {
      FAIL(__func__, "[%lu] -> Expected darr.available > %lu but %lu\n", i,
           tests[i].used + tests[i].n, darr.available);
      assert(false);
    }
    free(darr.data);
  }
}

TEST_SUITE(test_lib_darr, CREATE_TEST(test_lib_darr_init),
           CREATE_TEST(test_lib_darr_ensure_capacity_expands),
           CREATE_TEST(test_lib_darr_ensure_capacity_prev_data),
           CREATE_TEST(test_lib_darr_append_byte),
           CREATE_TEST(test_lib_darr_append_bytes),

);

#endif
