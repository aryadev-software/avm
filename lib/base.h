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

#define _DEFAULT_SOURCE
#include <endian.h>
#include <stdint.h>

/* Basic macros for a variety of uses.  Quite self explanatory. */
#define ARR_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))
#define MAX(a, b)    ((a) > (b) ? (a) : (b))
#define MIN(a, b)    ((a) > (b) ? (b) : (a))
#define TERM_GREEN   "\e[0;32m"
#define TERM_YELLOW  "\e[0;33m"
#define TERM_RED     "\e[0;31m"
#define TERM_RESET   "\e[0;0m"

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

typedef u8 byte;
typedef i8 s_byte;
typedef u32 hword;
typedef i32 s_hword;
typedef u64 word;
typedef i64 s_word;

/* Macros for the sizes of common base data types. */
#define HWORD_SIZE  sizeof(hword)
#define SHWORD_SIZE sizeof(s_hword)
#define WORD_SIZE   sizeof(word)
#define SWORD_SIZE  sizeof(s_word)

/** Union for all basic data types in the virtual machine.
 */
typedef union
{
  byte as_byte;
  s_byte as_char;
  hword as_hword;
  s_hword as_int;
  word as_word;
  s_word as_long;
} data_t;

/** Enum of type tags for the data_t structure to provide context.
 */
typedef enum
{
  DATA_TYPE_NIL = 0,
  DATA_TYPE_BYTE,
  DATA_TYPE_HWORD,
  DATA_TYPE_WORD,
} data_type_t;

/* Some macros for constructing data_t instances quickly. */
#define DBYTE(BYTE)   ((data_t){.as_byte = (BYTE)})
#define DHWORD(HWORD) ((data_t){.as_hword = (HWORD)})
#define DWORD(WORD)   ((data_t){.as_word = (WORD)})

/** Safely subtract SUB from W, where both are words (64 bit integers).
 *
 * In case of underflow (i.e. where W - SUB < 0) returns 0 instead of
 * the underflowed result.
 */
#define WORD_SAFE_SUB(W, SUB) ((W) > (SUB) ? ((W) - (SUB)) : 0)

/** Return the Nth byte of WORD
 * N should range from 0 to 7 as there are 8 bytes in a word.
 */
#define WORD_NTH_BYTE(WORD, N) (((WORD) >> ((N) * 8)) & 0xFF)

/** Return the Nth half word of WORD
 * N should range from 0 to 1 as there are 2 half words in a word
 */
#define WORD_NTH_HWORD(WORD, N) (((WORD) >> ((N) * 2)) & 0xFFFFFFFF)

/** Convert a buffer of bytes to a half word
 * We assume the buffer of bytes are in virtual machine byte code
 * format (big endian) and that they are at least HWORD_SIZE in
 * size.
 */
hword convert_bytes_to_hword(byte *buffer);

/** Convert a half word into a VM byte code format bytes (big endian)
 * @param h: Half word to convert
 * @param buffer: Buffer to store into.  We assume the buffer has at
 * least HWORD_SIZE space.
 */
void convert_hword_to_bytes(hword h, byte *buffer);

/** Convert a buffer of bytes to a word
 * We assume the buffer of bytes are in virtual machine byte code
 * format (big endian) and that they are at least WORD_SIZE in
 * size.
 */
word convert_bytes_to_word(byte *);

/** Convert a word into a VM byte code format bytes (big endian)
 * @param w: Word to convert
 * @param buffer: Buffer to store into.  We assume the buffer has at
 * least WORD_SIZE space.
 */
void convert_word_to_bytes(word w, byte *buffer);

#endif
