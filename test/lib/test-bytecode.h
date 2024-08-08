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
 * Description: Tests for bytecode.h
 */

#ifndef TEST_BYTECODE_H
#define TEST_BYTECODE_H

#include "../testing.h"
#include "lib/base.h"
#include <lib/bytecode.h>
#include <lib/inst-macro.h>

#include <string.h>

void test_lib_bytecode_bytecode_read_bytes_general(void)
{
  const struct TestCase
  {
    word_t size, read_size, start;
  } tests[] = {
      {10, 5, 2},
      {100, 50, 25},
      {1000, 500, 250},
      {1000, 1000, 0},
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
        .data = buffer, .available = test.size, .used = test.start};

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
    else if (bytecode.used != test.read_size + test.start)
    {
      FAIL(__func__, "[%lu] -> Expected cursor=%lu, got %lu\n", i,
           test.read_size + test.start, bytecode.used);
      assert(false);
    }
  }
}

void test_lib_bytecode_bytecode_read_bytes_error(void)
{
  const struct TestCase
  {
    word_t size, cursor, read_size;
  } tests[] = {
      {10, 10, 1},
      {100, 50, 100},
      {1000, 75, 2000},
  };
  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const struct TestCase test = tests[i];
    byte_t buffer[test.size];
    for (size_t j = 0; j < test.size; ++j)
      buffer[j] = rand() % 255;

    bytecode_t bytecode = {
        .data = buffer, .available = test.size, .used = test.cursor};

    byte_t *got = bytecode_read_bytes(&bytecode, test.read_size);

#if VERBOSE > 1
    INFO(__func__, "Testing(size=%lu, cursor=%lu, reading %lu)\n", test.size,
         test.cursor, test.read_size);
#endif

    if (got != NULL)
    {
      FAIL(__func__, "[%lu] -> Expected NULL, got %p\n", i, got);
      assert(false);
    }
    else if (bytecode.used != test.cursor)
    {
      FAIL(__func__, "[%lu] -> Expected cursor=%lu, got %lu\n", i, test.cursor,
           bytecode.used);
      assert(false);
    }
  }
}

void test_lib_bytecode_bytecode_read_word_general(void)
{
  const struct TestCase
  {
    word_t input;
  } tests[] = {
      {0x89ABCDEF},
      {0x12345678},
      {0xFFFFFFFFF0000000},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const struct TestCase test = tests[i];
    byte_t bytes[WORD_SIZE];
    memcpy(bytes, &test.input, WORD_SIZE);
    convert_bytes_le(bytes, WORD_SIZE);
    bytecode_t buffer = {.data = bytes, .used = 0, .available = WORD_SIZE};
    word_t got        = 0;
    bool success      = bytecode_read_word(&buffer, &got);

#if VERBOSE > 1
    INFO(__func__, "Testing(input=0x%lX)\n", test.input);
#endif
    if (!success)
    {
      FAIL(__func__, "[%lu] -> Couldn't read buffer!\n", i);
      assert(false);
    }
    else if (got != test.input)
    {
      FAIL(__func__, "[%lu] -> Expected %lu got %lu\n", i, test.input, got);
      assert(false);
    }
  }
}

void test_lib_bytecode_bytecode_read_word_error(void)
{
  const struct TestCase
  {
    word_t size, cursor;
  } tests[] = {
      {7, 0},
      {100, 93},
      {1000, 993},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const struct TestCase test = tests[i];
    byte_t bytes[test.size];
    bytecode_t buffer = {
        .data = bytes, .used = test.cursor, .available = test.size};

    word_t got   = 0;
    bool success = bytecode_read_word(&buffer, &got);

#if VERBOSE > 1
    INFO(__func__, "Testing(size=%lu, cursor=%lu)\n", test.size, test.cursor);
#endif
    if (success)
    {
      FAIL(__func__, "[%lu] -> Unexpected success?\n", i);
      assert(false);
    }
  }
}

static_assert(NUMBER_OF_OPCODES == 30,
              "test_lib_bytecode_bytecode_read_inst_* out of date");

void test_lib_bytecode_bytecode_read_inst_nullary(void)
{
  const struct TestCase
  {
    opcode_t opcode;
    inst_t expected;
  } tests[] = {{OP_NOOP, INST_NOOP}, {OP_HALT, INST_HALT}};

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const struct TestCase test = tests[i];
    byte_t bytes[]             = {test.opcode};
    bytecode_t buffer          = {
                 .data = bytes, .used = 0, .available = ARR_SIZE(bytes)};

    inst_t got   = {0};
    bool success = bytecode_read_inst(&buffer, &got);

#if VERBOSE > 1
    INFO(__func__, "Testing(opcode=%s, ", opcode_as_cstr(test.opcode));
    printf("expected=");
    inst_print(stdout, test.expected);
    printf(")\n");
#endif
    if (!success)
    {
      FAIL(__func__, "[%lu] -> Couldn't read buffer\n", i);
      assert(false);
    }
    else if (memcmp(&got, &test.expected, sizeof(got)) != 0)
    {
      FAIL(__func__, "[%lu] -> Expected ", i);
      inst_print(stderr, test.expected);
      fprintf(stderr, ", got ");
      inst_print(stderr, got);
      fprintf(stderr, "\n");
      assert(false);
    }
  }
}

void test_lib_bytecode_bytecode_read_inst_unary(void)
{
  word_t n = 0x1123456789abcdef;
  const struct TestCase
  {
    opcode_t opcode;
    inst_t expected;
  } tests[] = {
      {OP_POP, INST_POP(n)},
      {OP_DUP, INST_DUP(n)},
      {OP_NOT, INST_NOT(n)},
      {OP_OR, INST_OR(n)},
      {OP_AND, INST_AND(n)},
      {OP_XOR, INST_XOR(n)},
      {OP_EQ, INST_EQ(n)},
      {OP_PLUS_UNSIGNED, INST_PLUS_UNSIGNED(n)},
      {OP_MULT_UNSIGNED, INST_MULT_UNSIGNED(n)},
      {OP_SUB_UNSIGNED, INST_SUB_UNSIGNED(n)},
      {OP_PLUS, INST_PLUS(n)},
      {OP_SUB, INST_SUB(n)},
      {OP_MULT, INST_MULT(n)},
      {OP_LT_UNSIGNED, INST_LT_UNSIGNED(n)},
      {OP_LTE_UNSIGNED, INST_LTE_UNSIGNED(n)},
      {OP_GT_UNSIGNED, INST_GT_UNSIGNED(n)},
      {OP_GTE_UNSIGNED, INST_GTE_UNSIGNED(n)},
      {OP_LT, INST_LT(n)},
      {OP_LTE, INST_LTE(n)},
      {OP_GT, INST_GT(n)},
      {OP_GTE, INST_GTE(n)},
      {OP_JUMP_ABS, INST_JUMP_ABS(n)},
      {OP_CALL, INST_CALL(n)},
  };

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const struct TestCase test  = tests[i];
    byte_t bytes[1 + WORD_SIZE] = {test.opcode};
    memcpy(bytes + 1, &n, WORD_SIZE);
    convert_bytes_le(bytes + 1, WORD_SIZE);
    bytecode_t buffer = {
        .data = bytes, .used = 0, .available = ARR_SIZE(bytes)};

    inst_t got   = {0};
    bool success = bytecode_read_inst(&buffer, &got);

#if VERBOSE > 1
    INFO(__func__, "Testing(opcode=%s, ", opcode_as_cstr(test.opcode));
    printf("expected=");
    inst_print(stdout, test.expected);
    printf(")\n");
#endif
    if (!success)
    {
      FAIL(__func__, "[%lu] -> Couldn't read buffer\n", i);
      assert(false);
    }
    else if (memcmp(&got, &test.expected, sizeof(got)) != 0)
    {
      FAIL(__func__, "[%lu] -> Expected ", i);
      inst_print(stderr, test.expected);
      fprintf(stderr, ", got ");
      inst_print(stderr, got);
      fprintf(stderr, "\n");
      assert(false);
    }
  }
}

void test_lib_bytecode_bytecode_read_inst_binary(void)
{
  const word_t n = 20000;

  byte_t ops[WORD_SIZE];
  const word_t op_word = 0xfedcba9876543211;
  memcpy(ops, &op_word, WORD_SIZE);
  convert_bytes_le(ops, WORD_SIZE);

  const struct TestCase
  {
    opcode_t opcode;
    inst_t expected;
  } tests[] = {{OP_PUSH_REGISTER, INST_PUSH_REGISTER(n, ops)},
               {OP_MOV, INST_MOV(n, ops)},
               {OP_JUMP_IF, INST_JUMP_IF(n, ops)}};

  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    const struct TestCase test              = tests[i];
    byte_t bytes[1 + WORD_SIZE + WORD_SIZE] = {test.opcode};
    memcpy(bytes + 1, &n, WORD_SIZE);
    convert_bytes_le(bytes + 1, WORD_SIZE);
    memcpy(bytes + 1 + WORD_SIZE, &ops, WORD_SIZE);
    bytecode_t buffer = {
        .data = bytes, .used = 0, .available = ARR_SIZE(bytes)};

    inst_t got   = {0};
    bool success = bytecode_read_inst(&buffer, &got);

#if VERBOSE > 1
    INFO(__func__, "Testing(opcode=%s, ", opcode_as_cstr(test.opcode));
    printf("expected=");
    inst_print(stdout, test.expected);
    printf(")\n");
#endif
    if (!success)
    {
      FAIL(__func__, "[%lu] -> Couldn't read buffer\n", i);
      assert(false);
    }
    else if (!(got.opcode == test.expected.opcode && got.n == test.expected.n &&
               !memcmp(got.operands, test.expected.operands, WORD_SIZE)))
    {
      FAIL(__func__, "[%lu] -> Expected ", i);
      inst_print(stderr, test.expected);
      fprintf(stderr, ", got ");
      inst_print(stderr, got);
      fprintf(stderr, "\n");
      assert(false);
    }
  }
}

void test_lib_bytecode_bytecode_read_inst_nary(void)
{
  const struct TestCase
  {
    word_t number_operands;
  } tests[] = {{1}, {10}, {100}, {1000}};
  for (size_t i = 0; i < ARR_SIZE(tests); ++i)
  {
    word_t n = tests[i].number_operands;
    byte_t ops[n];
    for (size_t j = 0; j < n; ++j)
      ops[j] = rand() % 255;

    byte_t bytes[1 + WORD_SIZE + n];
    bytes[0] = OP_PUSH;
    memcpy(bytes + 1, &n, WORD_SIZE);
    convert_bytes_le(bytes + 1, WORD_SIZE);
    memcpy(bytes + 1 + WORD_SIZE, ops, n);

    bytecode_t buffer = {
        .data = bytes, .used = 0, .available = ARR_SIZE(bytes)};

    inst_t expected = INST_PUSH(n, ops);
    inst_t got      = {0};
    bool success    = bytecode_read_inst(&buffer, &got);
#if VERBOSE > 1
    INFO(__func__, "Testing(size=%lu)\n", n);
#endif
    if (!success)
    {
      FAIL(__func__, "[%lu] -> Couldn't read buffer\n", i);
      assert(false);
    }
    else if (!(got.opcode == expected.opcode && got.n == expected.n &&
               !memcmp(got.operands, expected.operands, n)))
    {
      FAIL(__func__, "[%lu] -> Expected ", i);
      inst_print(stderr, expected);
      fprintf(stderr, ", got ");
      inst_print(stderr, got);
      fprintf(stderr, "\n");
      assert(false);
    }
  }
}

TEST_SUITE(test_lib_bytecode,
           CREATE_TEST(test_lib_bytecode_bytecode_read_bytes_general),
           CREATE_TEST(test_lib_bytecode_bytecode_read_bytes_error),
           CREATE_TEST(test_lib_bytecode_bytecode_read_word_general),
           CREATE_TEST(test_lib_bytecode_bytecode_read_word_error),
           CREATE_TEST(test_lib_bytecode_bytecode_read_inst_nullary),
           CREATE_TEST(test_lib_bytecode_bytecode_read_inst_unary),
           CREATE_TEST(test_lib_bytecode_bytecode_read_inst_binary),
           CREATE_TEST(test_lib_bytecode_bytecode_read_inst_nary), );

#endif
