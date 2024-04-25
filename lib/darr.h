/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Dynamically sized byte array
 */

#ifndef DARR_H
#define DARR_H

#include <stdio.h>
#include <stdlib.h>

#include "./base.h"

/**
 * A dynamically sized buffer of bytes which may be used for a
 * variety of purposes.
 * @prop data: Buffer of bytes (may be reallocated)
 * @prop used: Number of bytes currently used
 * @prop available: Number of bytes currently allocated
 */
typedef struct
{
  byte_t *data;
  size_t used, available;
} darr_t;

/* Some useful constants for dynamic array work. */
#define DARR_DEFAULT_SIZE 8
#define DARR_REALLOC_MULT 1.5

/** Get the INDth item in a darr, where the buffer of bytes is
 * considerd an array of type TYPE.
 * Unsafe operation as safety checks are not done (in particular if
 * the dynamic array has IND items or is big enough to store an
 * element of TYPE) so it is presumed the caller will.
 */
#define DARR_AT(TYPE, DARR_DATA, IND) ((TYPE *)(DARR_DATA))[(IND)]

/** Initialise a dynamic array (darr) with n elements.
 * If n == 0 then initialise with DARR_DEFAULT_SIZE elements.
 */
void darr_init(darr_t *darr, size_t n);

/** Ensure the dynamic array (darr) has at least n elements free.
 * If the dynamic array has less than n elements free it will
 * reallocate.
 */
void darr_ensure_capacity(darr_t *darr, size_t n);

/** Append a byte (b) to the dynamic array (darr).
 * If the dynamic array doesn't have enough space it will reallocate
 * to ensure it can fit it in.
 */
void darr_append_byte(darr_t *darr, byte_t b);

/** Append an array of n bytes (b) to the dynamic array (darr).
 * If the dynamic array doesn't have enough space to fit all n bytes
 * it will reallocate to ensure it can fit it in.
 */
void darr_append_bytes(darr_t *darr, byte_t *b, size_t n);

/** Safely get the nth byte of the dynamic array (darr)
 * If the dynamic array has less than n bytes used, it will return 0
 * as a default value.
 */
byte_t darr_at(darr_t *darr, size_t n);

/** Write the dynamic array (darr) to the file pointer (fp) as a
 * buffer of bytes.
 * Assumes fp is a valid file pointer and in write mode.
 */
void darr_write_file(darr_t *, FILE *);

/** Read a file pointer (fp) in its entirety, converting the bytes
 * into a tightly fitted dynamic array.
 * Say the file pointer is a file of n bytes.  Then the dynamic array
 * returned will have available set to n and used set to 0.
 */
darr_t darr_read_file(FILE *);

#endif
