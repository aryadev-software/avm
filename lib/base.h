/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Basic types and routines
 */

#ifndef BASE_H
#define BASE_H

#include <stdint.h>
#include <stdio.h>

/* Basic macros for a variety of uses.  Quite self explanatory. */
#define ARR_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))
#define MAX(a, b)    ((a) > (b) ? (a) : (b))
#define MIN(a, b)    ((a) > (b) ? (b) : (a))
#define TERM_GREEN   "\033[32m"
#define TERM_YELLOW  "\033[33m"
#define TERM_RED     "\033[31m"
#define TERM_RESET   "\033[0m"

#define MESSAGE(FILE, COLOUR, NAME, FORMAT, ...) \
  fprintf(FILE, "[" COLOUR "%s" TERM_RESET "]: " FORMAT, NAME, __VA_ARGS__)

#define INFO(NAME, FORMAT, ...) \
  MESSAGE(stdout, TERM_YELLOW, NAME, FORMAT, __VA_ARGS__)
#define FAIL(NAME, FORMAT, ...) \
  MESSAGE(stderr, TERM_RED, NAME, FORMAT, __VA_ARGS__)
#define SUCCESS(NAME, FORMAT, ...) \
  MESSAGE(stdout, TERM_GREEN, NAME, FORMAT, __VA_ARGS__)

// Flags for program behaviour (usually related to printing)
#ifndef VERBOSE
#define VERBOSE 0
#endif
#ifndef PRINT_HEX
#define PRINT_HEX 0
#endif

/* Ease of use aliases for numeric types */
typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef float f32;
typedef uint64_t u64;
typedef int64_t i64;
typedef double f64;

typedef u8 byte_t;
typedef i8 sbyte_t;
typedef u16 short_t;
typedef i16 sshort_t;
typedef u32 hword_t;
typedef i32 shword_t;
typedef u64 word_t;
typedef i64 sword_t;

/* Macros for the sizes of common base data types. */
#define BYTE_SIZE  1
#define SHORT_SIZE sizeof(short_t)
#define HWORD_SIZE sizeof(hword_t)
#define WORD_SIZE  sizeof(word_t)

/* Macros for the bounds of base types */
#define BYTE_MAX UINT8_MAX
#define CHAR_MAX INT8_MAX
#define CHAR_MIN INT8_MIN

#define HWORD_MAX UINT32_MAX
#define INT_MAX   INT32_MAX
#define INT_MIN   INT32_MIN

#define WORD_MAX UINT64_MAX
#define LONG_MAX INT64_MAX
#define LONG_MIN INT64_MIN

/**
   @brief Union for all basic data types in the virtual machine.  Allows for
   easy type punning.
*/
typedef union
{
  byte_t as_byte;
  sbyte_t as_sbyte;
  short_t as_short;
  sshort_t as_sshort;
  hword_t as_hword;
  shword_t as_shword;
  word_t as_word;
  sword_t as_sword;
} data_t;

/* Some macros for constructing data_t instances quickly. */
#define DBYTE(BYTE)   ((data_t){.as_byte = (BYTE)})
#define DSHORT(SHORT) ((data_t){.as_short = (SHORT)})
#define DHWORD(HWORD) ((data_t){.as_hword = (HWORD)})
#define DWORD(WORD)   ((data_t){.as_word = (WORD)})

// Macro to determine if the current machine is little endian
#ifndef LITTLE_ENDIAN
static const hword_t __i = 0xFFFF0000;
#define LITTLE_ENDIAN ((*((byte_t *)&__i)) == 0)
#endif

/**
   @brief Safely subtract SUB from W, where both are words (64 bit integers).

   @details In case of underflow (i.e. where W - SUB < 0) returns 0 instead of
   the underflowed result.
*/
#define WORD_SAFE_SUB(W, SUB) ((W) > (SUB) ? ((W) - (SUB)) : 0)

/**
   @brief Return the Nth byte of WORD.

   @details N should range from 0 to 7 as there are 8 bytes in a word.
*/
#define WORD_NTH_BYTE(WORD, N) (((WORD) >> ((N) * 8)) & 0xFF)

/**
   @brief Return the Nth short of WORD.

   @details N should range from 0 to 3 as there are 4 shorts in a word
*/
#define WORD_NTH_SHORT(WORD, N) (((WORD) >> ((N) * 16)) & 0xFFFF)

/**
   @brief Return the Nth half word of WORD.

   @details N should range from 0 to 1 as there are 2 half words in a word
*/
#define WORD_NTH_HWORD(WORD, N) (((WORD) >> ((N) * 32)) & 0xFFFFFFFF)

/**
   @brief Reverse an array of bytes in place.

   @param[bytes] Array of bytes to reverse
   @param[size] Size of array `bytes`.
 */
void byteswap(byte_t *bytes, size_t size);

/**
   @brief Convert a buffer of bytes to and from Little Endian in place.

   @param[buffer] Buffer of bytes to convert.
   @param[buffer_size] Size of buffer to convert.
 */
void convert_bytes_le(byte_t *buffer, size_t buffer_size);

/**
   @brief Print a byte array in hex.

   @param[fp] File to write to.
   @param[bytes] Byte array to print.
   @param[size_bytes] Size of byte array.
 */
void print_byte_array(FILE *fp, const byte_t *bytes, size_t size_bytes);

#endif
