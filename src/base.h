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

typedef uint64_t u64;
typedef uint32_t u32;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

typedef uint8_t byte;
typedef u64 word;

typedef union
{
  byte as_byte;
  word as_word;
  f64 as_float;
} data_t;

typedef enum
{
  DATA_TYPE_NIL   = 0,
  DATA_TYPE_BYTE  = 1,
  DATA_TYPE_WORD  = 3,
  DATA_TYPE_FLOAT = 5,
} data_type_t;

#define DBYTE(BYTE)   ((data_t){.as_byte = (BYTE)})
#define DWORD(WORD)   ((data_t){.as_word = (WORD)})
#define DFLOAT(FLOAT) ((data_t){.as_float = (FLOAT)})

#define WORD_SIZE  sizeof(word)
#define FLOAT_SIZE sizeof(f64)

#endif
