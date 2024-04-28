/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2024-04-28
 * Author: Aryadev Chavali
 * Description: Tests for base.h
 */

#ifndef TEST_BASE_H
#define TEST_BASE_H

#include <lib/base.h>
#include <testing.h>

#include <string.h>

void test_lib_base_word_safe_sub(void)
{
  const struct TestCase
  {
    word_t a;
    word_t b;
    word_t expected;
  } tests[] = {
      {0xFF, 0, 0xFF},
      {1LU << 32, 1LU << 31, 1LU << 31},
      {0, 100, 0},
      {~(1LU << 62), ~(1LU << 63), 1LU << 62},
  };
  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const struct TestCase test = tests[i];
    const word_t res           = test.expected;
    const word_t got           = WORD_SAFE_SUB(test.a, test.b);
#if VERBOSE > 1
    INFO(__func__, "Testing(a=0x%lX, b=0x%lX) = 0x%lX\n", test.a, test.b,
         test.expected);
#endif
    if (got != res)
    {
      FAIL(__func__, "[%lu] -> Expected 0x%lX got 0x%lX\n", i, res, got);
      assert(false);
    }
  }
}

void test_lib_base_word_nth_byte(void)
{
  const struct TestCase
  {
    word_t word;
    size_t n;
    byte_t expected;
  } tests[] = {
      {0, 0, 0},
      {0x0123456789abcdef, 0, 0xef},
      {0x0123456789abcdef, 1, 0xcd},
      {0x0123456789abcdef, 2, 0xab},
      {0x0123456789abcdef, 3, 0x89},
      {0x0123456789abcdef, 4, 0x67},
      {0x0123456789abcdef, 5, 0x45},
      {0x0123456789abcdef, 6, 0x23},
      {0x0123456789abcdef, 7, 0x01},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const word_t w        = tests[i].word;
    const size_t n        = tests[i].n;
    const byte_t expected = tests[i].expected;
#if VERBOSE > 1
    INFO(__func__, "Testing(0x%lX, %lu) = 0x%X\n", w, n, expected);
#endif
    const byte_t got = WORD_NTH_BYTE(w, n);
    if (got != expected)
    {
      FAIL(__func__, "[%lu] -> Expected 0x%X got 0x%X\n", i, expected, got);
      assert(false);
    }
  }
}

void test_lib_base_word_nth_hword(void)
{
  const struct TestCase
  {
    word_t word;
    size_t n;
    hword_t expected;
  } tests[] = {
      {0, 0, 0},
      {0x0123456789abcdef, 0, 0x89abcdef},
      {0x0123456789abcdef, 1, 0x01234567},
      {~(1LU << 63), 0, 0xFFFFFFFF},
      {~(1LU << 63), 1, 0x7FFFFFFF},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const word_t w         = tests[i].word;
    const size_t n         = tests[i].n;
    const hword_t expected = tests[i].expected;
#if VERBOSE > 1
    INFO(__func__, "Testing(0x%lX, %lu) = 0x%X\n", w, n, expected);
#endif
    const hword_t got = WORD_NTH_HWORD(w, n);
    if (got != expected)
    {
      FAIL(__func__, "[%lu] -> Expected 0x%X got 0x%X\n", i, expected, got);
      assert(false);
    }
  }
}

void test_lib_base_hword_byteswap(void)
{
  struct TestCase
  {
    hword_t sample, expected;
  } tests[] = {
      {0, 0},
      {0xFF0000AA, 0xAA0000FF},
      {0x89ABCDEF, 0xEFCDAB89},
      {0x12FAAF21, 0x21AFFA12},
      {0xEFBEADDE, 0xDEADBEEF},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    hword_t test     = tests[i].sample;
    hword_t expected = tests[i].expected;
#if VERBOSE > 1
    INFO(__func__, "Testing 0x%X = 0x%X\n", test, expected);
#endif
    hword_t got = hword_byteswap(test);
    if (expected != got)
    {
      FAIL(__func__, "[%lu] -> Expected 0x%X got 0x%X\n", i, expected, got);
      assert(false);
    }
  }
}

void test_lib_base_word_byteswap(void)
{
  struct TestCase
  {
    word_t sample, expected;
  } tests[] = {
      {0, 0},
      {0xFF000000000000AA, 0xAA000000000000FF},
      {0x1023456789ABCDEF, 0xEFCDAB8967452310},
      {0xAAAAAAAAFFFFFFFF, 0xFFFFFFFFAAAAAAAA},
      {0x89ABCDEFFEDCBA98, 0x98BADCFEEFCDAB89},
      {0x1000EFBEADDE0011, 0x1100DEADBEEF0010},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    word_t test     = tests[i].sample;
    word_t expected = tests[i].expected;
#if VERBOSE > 1
    INFO(__func__, "Testing 0x%lX = 0x%lX\n", test, expected);
#endif
    word_t got = word_byteswap(test);
    if (expected != got)
    {
      FAIL(__func__, "[%lu] -> Expected 0x%lX got 0x%lX\n", i, expected, got);
      assert(false);
    }
  }
}

void test_lib_base_bytes_to_hword(void)
{
  const struct TestCase
  {
    byte_t bytes[4];
    hword_t expected;
  } tests[] = {{{0, 0, 0, 0}, 0},
               {{0xFF, 0xFF, 0xFF, 0xFF}, HWORD_MAX},
               {{1, 0, 0, 0}, 1},
               {{0, 0, 0, 0b10000000}, 1 << 31},
               {{0x89, 0xab, 0xcd, 0xef}, 0xefcdab89}};

  const size_t n = size_byte_array_to_string(4);
  char str[n];
  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    memset(str, 0, n);
    const hword_t got = convert_bytes_to_hword(tests[i].bytes);
    byte_array_to_string(tests[i].bytes, 4, str);
#if VERBOSE > 1
    INFO(__func__, "Testing%s = 0x%X\n", str, tests[i].expected);
#endif
    if (tests[i].expected != got)
    {
      FAIL(__func__, "[%lu] -> Expected 0x%X got 0x%X\n", i, tests[i].expected,
           got);
      assert(false);
    }
  }
}

void test_lib_base_bytes_to_word(void)
{
  const struct TestCase
  {
    byte_t bytes[8];
    word_t expected;
  } tests[] = {
      {{0, 0, 0, 0}, 0},
      {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, WORD_MAX},
      {{0x01, 0, 0, 0, 0, 0, 0, 0}, 1},
      {{0, 0, 0, 0, 0, 0, 0, 0b10000000}, 1LU << 63},
      {{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}, 0xefcdab8967452301}};

  const size_t n = size_byte_array_to_string(8);
  char str[n];
  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    memset(str, 0, n);
    byte_array_to_string(tests[i].bytes, 8, str);
#if VERBOSE > 1
    INFO(__func__, "Testing%s = 0x%lX\n", str, tests[i].expected);
#endif
    const word_t got = convert_bytes_to_word(tests[i].bytes);
    if (tests[i].expected != got)
    {
      FAIL(__func__, "[%lu] -> Expected 0x%lx got 0x%lx\n", i,
           tests[i].expected, got);
      assert(false);
    }
  }
}

void test_lib_base_hword_to_bytes(void)
{
  const struct TestCase
  {
    hword_t sample;
    byte_t expected[4];
  } tests[] = {{0, {0, 0, 0, 0}},
               {HWORD_MAX, {0xFF, 0xFF, 0xFF, 0xFF}},
               {1, {1, 0, 0, 0}},
               {1 << 31, {0, 0, 0, 0x80}},
               {0xefcdab89, {0x89, 0xab, 0xcd, 0xef}}};

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    byte_t buffer[4] = {0};
    convert_hword_to_bytes(tests[i].sample, buffer);
#if VERBOSE > 1
    const size_t n = size_byte_array_to_string(4);
    char str_expected[n];
    byte_array_to_string(tests[i].expected, 4, str_expected);
    INFO(__func__, "Testing(0x%X) = %s\n", tests[i].sample, str_expected);
#endif
    if (memcmp(buffer, tests[i].expected, 4) != 0)
    {
      const size_t n = size_byte_array_to_string(4);
      char str_expected[n];
      char str_got[n];
      byte_array_to_string(tests[i].expected, 4, str_expected);
      byte_array_to_string(buffer, 4, str_got);
      FAIL(__func__, "[%lu] -> Expected %s got %s\n", i, str_expected, str_got);
      assert(false);
    }
  }
}

void test_lib_base_word_to_bytes(void)
{
  const struct TestCase
  {
    word_t sample;
    byte_t expected[8];
  } tests[] = {
      {0, {0, 0, 0, 0}},
      {WORD_MAX, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
      {1, {0x01, 0, 0, 0, 0, 0, 0, 0}},
      {1LU << 63, {0, 0, 0, 0, 0, 0, 0, 0b10000000}},
      {0xefcdab8967452301, {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}}};

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    byte_t buffer[8] = {0};
    convert_word_to_bytes(tests[i].sample, buffer);
#if VERBOSE > 1
    const size_t n = size_byte_array_to_string(8);
    char str_expected[n];
    byte_array_to_string(tests[i].expected, 8, str_expected);
    INFO(__func__, "Testing(0x%lx) = %s\n", tests[i].sample, str_expected);
#endif
    if (memcmp(buffer, tests[i].expected, 8) != 0)
    {
      const size_t n = size_byte_array_to_string(8);
      char str_expected[n];
      char str_got[n];
      byte_array_to_string(tests[i].expected, 8, str_expected);
      byte_array_to_string(buffer, 8, str_got);
      FAIL(__func__, "[%lu] -> Expected %s got %s\n", i, str_expected, str_got);
      assert(false);
    }
  }
}

TEST_SUITE(test_lib_base, CREATE_TEST(test_lib_base_word_safe_sub),
           CREATE_TEST(test_lib_base_word_nth_byte),
           CREATE_TEST(test_lib_base_word_nth_hword),
           CREATE_TEST(test_lib_base_hword_byteswap),
           CREATE_TEST(test_lib_base_word_byteswap),
           CREATE_TEST(test_lib_base_bytes_to_hword),
           CREATE_TEST(test_lib_base_bytes_to_word),
           CREATE_TEST(test_lib_base_hword_to_bytes),
           CREATE_TEST(test_lib_base_word_to_bytes), );

#endif
