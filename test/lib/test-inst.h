/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * details.

 * You may distribute and modify this code under the terms of the GNU General
 * Public License Version 2, which you should have received a copy of along with
 * this program.  If not, please go to <https://www.gnu.org/licenses/>.

 * Created: 2024-07-14
 * Author: Aryadev Chavali
 * Description: Tests for inst.h
 */

#ifndef TEST_INST_H
#define TEST_INST_H

#include "../testing.h"
#include <lib/inst.h>

#include <string.h>

void test_lib_inst_bytecode_read_bytes(void)
{
  const struct TestCase
  {
    word_t size, read_size, start;
  } tests[] = {
      {10, 5, 2},
      {100, 50, 25},
      {1000, 500, 250},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const struct TestCase test = tests[i];
    byte_t buffer[test.size];
    for (size_t j = 0; j < test.size; ++j)
      buffer[j] = rand() % 255;

    byte_t expected[test.read_size];
    memcpy(expected, buffer + test.start, test.read_size);

    bytecode_t bytecode = {
        .bytes = buffer, .size = test.size, .cursor = test.start};

    byte_t *got = bytecode_read_bytes(&bytecode, test.read_size);

#if VERBOSE > 1
    INFO(__func__, "Testing(size=%lu, start=%lu, reading %lu)\n", test.size,
         test.start, test.read_size);
#endif
    if (memcmp(got, expected, test.read_size) != 0)
    {
      FAIL(__func__, "[%lu] -> Memory read vs memory expected\n", i);
      assert(false);
    }
    else if (bytecode.cursor != test.read_size + test.start)
    {
      FAIL(__func__, "[%lu] -> Expected cursor=%lu, got %lu\n", i,
           test.read_size + test.start, bytecode.cursor);
      assert(false);
    }
  }
}

TEST_SUITE(test_lib_inst, CREATE_TEST(test_lib_inst_bytecode_read_bytes));

#endif
