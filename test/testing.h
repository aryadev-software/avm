/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2024-04-28
 * Author: Aryadev Chavali
 * Description: Base library for creating tests
 */

#ifndef TESTING_H
#define TESTING_H

#include <assert.h>
#include <lib/base.h>
#include <stdbool.h>
#include <stdio.h>

#define MESSAGE(FILE, COLOUR, NAME, FORMAT, ...) \
  fprintf(FILE, "\t[" COLOUR "%s" TERM_RESET "]: " FORMAT, NAME, __VA_ARGS__)

#define INFO(NAME, FORMAT, ...) \
  MESSAGE(stdout, TERM_YELLOW, NAME, FORMAT, __VA_ARGS__)
#define FAIL(NAME, FORMAT, ...) \
  MESSAGE(stderr, TERM_RED, NAME, FORMAT, __VA_ARGS__)
#define SUCCESS(NAME, FORMAT, ...) \
  MESSAGE(stdout, TERM_GREEN, NAME, FORMAT, __VA_ARGS__)

typedef void (*test_fn)(void);
struct Test
{
  const char *name;
  test_fn src;
};

#define CREATE_TEST(NAME)      \
  (struct Test)                \
  {                            \
    .name = #NAME, .src = NAME \
  }

#define TEST_SUITE(NAME, ...) struct Test NAME[] = {__VA_ARGS__}
#define RUN_TEST_SUITE(SUITE)                         \
  INFO(#SUITE, "%s", "Starting test suite...\n");     \
  for (size_t i = 0; i < ARR_SIZE(SUITE); ++i)        \
  {                                                   \
    SUITE[i].src();                                   \
    SUCCESS(SUITE[i].name, "%s\n", "Test succeeded"); \
  }                                                   \
  SUCCESS(#SUITE, "%s", "Finished test suite!\n")

#endif
