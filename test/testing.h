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

#define MESSAGE(COLOUR, NAME, FORMAT, ...) \
  printf("\t[" COLOUR "%s" TERM_RESET "]: " FORMAT, NAME, __VA_ARGS__)

#define INFO(NAME, FORMAT, ...) MESSAGE(TERM_YELLOW, NAME, FORMAT, __VA_ARGS__)
#define FAIL(NAME, FORMAT, ...) MESSAGE(TERM_RED, NAME, FORMAT, __VA_ARGS__)
#define SUCCESS(NAME, FORMAT, ...) \
  MESSAGE(TERM_GREEN, NAME, FORMAT, __VA_ARGS__)

typedef void (*test_fn)(void);

#define TEST_SUITE(NAME, ...) test_fn NAME[] = {__VA_ARGS__}
#define RUN_TEST_SUITE(NAME)                     \
  INFO(#NAME, "%s", "Starting test suite...\n"); \
  for (size_t i = 0; i < ARR_SIZE(NAME); ++i)    \
    NAME[i]();                                   \
  SUCCESS(#NAME, "%s", "Finished test suite!\n")

#endif
