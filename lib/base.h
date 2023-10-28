/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the GPLv2
 * license.  You should have received a copy of the GPLv2 license with
 * this file.  If not, please write to: aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Basic types and routines
 */

#ifndef BASE_H
#define BASE_H

#include <stdint.h>

#define ARR_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))
#define MAX(a, b)    ((a) > (b) ? (a) : (b))
#define MIN(a, b)    ((a) > (b) ? (b) : (a))

// Flags
#ifndef VERBOSE
#define VERBOSE 0
#endif

typedef uint64_t u64;
typedef uint32_t u32;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

typedef uint8_t byte;
typedef u32 hword;
typedef u64 word;

typedef union
{
  byte as_byte;
  hword as_hword;
  word as_word;
} data_t;

typedef enum
{
  DATA_TYPE_NIL = 0,
  DATA_TYPE_BYTE,
  DATA_TYPE_HWORD,
  DATA_TYPE_WORD,
} data_type_t;

#define DBYTE(BYTE)           ((data_t){.as_byte = (BYTE)})
#define DHWORD(HWORD)         ((data_t){.as_hword = (HWORD)})
#define DWORD(WORD)           ((data_t){.as_word = (WORD)})
#define WORD_SAFE_SUB(W, SUB) ((W) > (SUB) ? ((W) - (SUB)) : 0)

#define HWORD_SIZE sizeof(hword)
#define WORD_SIZE  sizeof(word)

// Macros to extract the nth byte or nth hword from a word
#define WORD_NTH_BYTE(WORD, N)  (((WORD) >> ((N)*8)) & 0xff)
#define WORD_NTH_HWORD(WORD, N) (((WORD) >> ((N)*2)) & 0xFFFFFFFF)

// Assume array contains 4 bytes.
hword convert_bytes_to_hword(byte *);
void convert_hword_to_bytes(hword, byte *);
// Assume array contains 8 bytes.
word convert_bytes_to_word(byte *);
void convert_word_to_bytes(word, byte *);

#endif
