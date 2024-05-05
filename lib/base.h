/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Basic types and routines
 */

#ifndef BASE_H
#define BASE_H

#include <stdint.h>

/* Basic macros for a variety of uses.  Quite self explanatory. */
#define ARR_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))
#define MAX(a, b)    ((a) > (b) ? (a) : (b))
#define MIN(a, b)    ((a) > (b) ? (b) : (a))
#define TERM_GREEN   "\033[32m"
#define TERM_YELLOW  "\033[33m"
#define TERM_RED     "\033[31m"
#define TERM_RESET   "\033[0m"

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
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef u8 byte_t;
typedef i8 char_t;
typedef u32 hword_t;
typedef i32 int_t;
typedef u64 word_t;
typedef i64 long_t;

/* Macros for the sizes of common base data types. */
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
   @brief Union for all basic data types in the virtual machine.
*/
typedef union
{
  byte_t as_byte;
  char_t as_char;
  hword_t as_hword;
  int_t as_int;
  word_t as_word;
  long_t as_long;
} data_t;

/**
   @brief Enum of type tags for the data_t structure to provide
   context.
*/
typedef enum
{
  DATA_TYPE_NIL = -1,
  DATA_TYPE_BYTE,
  DATA_TYPE_HWORD,
  DATA_TYPE_WORD,
} data_type_t;

/* Some macros for constructing data_t instances quickly. */
#define DBYTE(BYTE)   ((data_t){.as_byte = (BYTE)})
#define DHWORD(HWORD) ((data_t){.as_hword = (HWORD)})
#define DWORD(WORD)   ((data_t){.as_word = (WORD)})

// Macro to determine little endian
#ifndef LITTLE_ENDIAN
static const hword_t __i = 0xFFFF0000;
#define LITTLE_ENDIAN ((*((byte_t *)&__i)) == 0)
#endif

/**
   @brief Safely subtract SUB from W, where both are words (64 bit
   integers).

   @details In case of underflow (i.e. where W - SUB < 0) returns 0
   instead of the underflowed result.
*/
#define WORD_SAFE_SUB(W, SUB) ((W) > (SUB) ? ((W) - (SUB)) : 0)

/**
   @brief Return the Nth byte of WORD.

   @details N should range from 0 to 7 as there are 8 bytes in a word.
*/
#define WORD_NTH_BYTE(WORD, N) (((WORD) >> ((N) * 8)) & 0xFF)

/**
   @brief Return the Nth half word of WORD.

   @details N should range from 0 to 1 as there are 2 half words in a
   word
*/
#define WORD_NTH_HWORD(WORD, N) (((WORD) >> ((N) * 32)) & 0xFFFFFFFF)

/**
   @brief Convert a buffer of bytes to a half word.

   @details We assume the buffer of bytes are in virtual machine byte
   code format (little endian) and that they are at least HWORD_SIZE
   in size.
*/
hword_t convert_bytes_to_hword(const byte_t *buffer);

/**
   @brief Convert a half word into a VM byte code format bytes (big
   endian)

   @param h: Half word to convert
   @param buffer: Buffer to store into.  We assume the buffer has at
   least HWORD_SIZE space.
*/
void convert_hword_to_bytes(const hword_t h, byte_t *buffer);

/**
   @brief Convert a buffer of bytes to a word.

   @details We assume the buffer of bytes are in virtual machine byte
   code format (little endian) and that they are at least WORD_SIZE in
   size.
*/
word_t convert_bytes_to_word(const byte_t *);

/**
   @brief Convert a word into a VM byte code format bytes (little
   endian)

   @param w: Word to convert

   @param buffer: Buffer to store into.  We assume the buffer has at
   least WORD_SIZE space.
*/
void convert_word_to_bytes(const word_t w, byte_t *buffer);

hword_t hword_byteswap(const hword_t h);

word_t word_byteswap(const word_t w);

#endif
