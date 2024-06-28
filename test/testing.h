/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
#include <stdlib.h>
#include <string.h>

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
  (const struct Test)          \
  {                            \
    .name = #NAME, .src = NAME \
  }

#define TEST_SUITE(NAME, ...) struct Test NAME[] = {__VA_ARGS__}
#if VERBOSE >= 1
#define RUN_TEST_SUITE(SUITE)                             \
  INFO("<" #SUITE ">", "%s", "Starting test suite...\n"); \
  for (size_t i = 0; i < ARR_SIZE(SUITE); ++i)            \
  {                                                       \
    SUITE[i].src();                                       \
    SUCCESS(SUITE[i].name, "%s\n", "Test passed");        \
  }                                                       \
  SUCCESS("<" #SUITE ">", "%s", "Test suite passed!\n")
#else
#define RUN_TEST_SUITE(SUITE)                             \
  INFO("<" #SUITE ">", "%s", "Starting test suite...\n"); \
  for (size_t i = 0; i < ARR_SIZE(SUITE); ++i)            \
    SUITE[i].src();                                       \
  SUCCESS("<" #SUITE ">", "%s", "Test suite passed!\n")
#endif

static size_t size_byte_array_to_string(const size_t n)
{
  return 3 + (4 * n) + (2 * (n - 1));
}

static void byte_array_to_string(const byte_t *bytes, size_t size_bytes,
                                 char *str)
{
  str[0]   = '{';
  size_t j = 1;
  for (size_t i = 0; i < size_bytes; ++i)
  {
    char buffer[8];
    int k    = i == size_bytes - 1 ? 0 : 2;
    size_t n = 4 + k;

    sprintf(buffer, "0x%s%X, ", (bytes[i] < 16 ? "_" : ""), bytes[i]);
    memcpy(str + j, buffer, n);
    j += n;
  }
  str[j]     = '}';
  str[j + 1] = '\0';
}

#endif
