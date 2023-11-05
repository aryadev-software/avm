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

typedef struct
{
  byte *data;
  size_t used, available;
} darr_t;

#define DARR_DEFAULT_SIZE             8
#define DARR_REALLOC_MULT             1.5
#define DARR_AT(TYPE, DARR_DATA, IND) ((TYPE *)(DARR_DATA))[(IND)]

void darr_init(darr_t *, size_t);
void darr_ensure_capacity(darr_t *, size_t);
void darr_append_byte(darr_t *, byte);
void darr_append_bytes(darr_t *, byte *, size_t);
byte darr_at(darr_t *, size_t);

void darr_write_file(darr_t *, FILE *);
darr_t darr_read_file(FILE *);

#endif
