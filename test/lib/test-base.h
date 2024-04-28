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

void testing_lib_bytes_to_hword(void)
{
  const byte_t tests[][4] = {{0, 0, 0, 0},
                             {0xFF, 0xFF, 0xFF, 0xFF},
                             {1},
                             {0, 0, 0, 0b10000000},
                             {0x89, 0xab, 0xcd, 0xef}};

  const hword_t expected[ARR_SIZE(tests)] = {0, HWORD_MAX, 1, 1 << 31,
                                             0xefcdab89};

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const hword_t got = convert_bytes_to_hword(tests[i]);
    if (expected[i] != got)
    {
      FAIL(__func__, "[%lu] -> Expected 0x%x got 0x%x\n", i, expected[i], got);
      assert(false);
    }
  }
  SUCCESS(__func__, "%s\n", "Test succeeded");
}

void testing_lib_bytes_to_word(void)
{
  const byte_t tests[][8] = {{0, 0, 0, 0},
                             {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                             {0x01, 0, 0, 0, 0, 0, 0, 0},
                             {0, 0, 0, 0, 0, 0, 0, 0b10000000},
                             {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}};

  const word_t expected[ARR_SIZE(tests)] = {0, WORD_MAX, 1, 1LU << 63,

                                            0xefcdab8967452301};
  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const word_t got = convert_bytes_to_word(tests[i]);
    if (expected[i] != got)
    {
      FAIL(__func__, "[%lu] -> Expected 0x%lx got 0x%lx\n", i, expected[i],
           got);
      assert(false);
    }
  }
  SUCCESS(__func__, "%s\n", "Test succeeded");
}

TEST_SUITE(test_lib, testing_lib_bytes_to_hword, testing_lib_bytes_to_word);

#endif
