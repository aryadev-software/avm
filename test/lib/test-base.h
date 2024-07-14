/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2024-04-28
 * Author: Aryadev Chavali
 * Description: Tests for base.h
 */

#ifndef TEST_BASE_H
#define TEST_BASE_H

#include "../testing.h"
#include <lib/base.h>

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
    INFO(__func__, "Testing(a=0x%lX, b=0x%lX) -> 0x%lX\n", test.a, test.b,
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
    INFO(__func__, "Testing(0x%lX, %lu) -> 0x%X\n", w, n, expected);
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
    INFO(__func__, "Testing(0x%lX, %lu) -> 0x%X\n", w, n, expected);
#endif
    const hword_t got = WORD_NTH_HWORD(w, n);
    if (got != expected)
    {
      FAIL(__func__, "[%lu] -> Expected 0x%X got 0x%X\n", i, expected, got);
      assert(false);
    }
  }
}

TEST_SUITE(test_lib_base, CREATE_TEST(test_lib_base_word_safe_sub),
           CREATE_TEST(test_lib_base_word_nth_byte),
           CREATE_TEST(test_lib_base_word_nth_hword), );

#endif
