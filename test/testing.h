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

#endif
