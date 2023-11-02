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

#define ARR_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))
#define MAX(a, b)    ((a) > (b) ? (a) : (b))
#define MIN(a, b)    ((a) > (b) ? (b) : (a))
#define TERM_GREEN   "\e[0;32m"
#define TERM_YELLOW  "\e[0;33m"
#define TERM_RED     "\e[0;31m"
#define TERM_RESET   "\e[0;0m"

// Flags
#ifndef VERBOSE
#define VERBOSE 0
#endif
#ifndef PRINT_HEX
#define PRINT_HEX 0
#endif

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

typedef union
{
  byte as_byte;
  s_byte as_char;
  hword as_hword;
  s_hword as_int;
  word as_word;
  s_word as_long;
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

#define HWORD_SIZE  sizeof(hword)
#define SHWORD_SIZE sizeof(s_hword)
#define WORD_SIZE   sizeof(word)
#define SWORD_SIZE  sizeof(s_word)

// Macros to extract the nth byte or nth hword from a word
#define WORD_NTH_BYTE(WORD, N)  (((WORD) >> ((N)*8)) & 0xFF)
#define WORD_NTH_HWORD(WORD, N) (((WORD) >> ((N)*2)) & 0xFFFFFFFF)

// Assume array contains 4 bytes.
hword convert_bytes_to_hword(byte *);
void convert_hword_to_bytes(hword, byte *);
// Assume array contains 8 bytes.
word convert_bytes_to_word(byte *);
void convert_word_to_bytes(word, byte *);

#endif
