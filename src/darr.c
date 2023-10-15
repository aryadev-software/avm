/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Dynamically sized byte array
 */

#include <malloc.h>
#include <string.h>

#include "./darr.h"

void darr_init(darr_t *darr, size_t size)
{
  if (size == 0)
    size = DARR_DEFAULT_SIZE;
  *darr = (darr_t){
      .data      = calloc(size, 1),
      .used      = 0,
      .available = size,
  };
}

void darr_ensure_capacity(darr_t *darr, size_t requested)
{
  if (darr->used + requested >= darr->available)
  {
    darr->available =
        MAX(darr->used + requested, darr->available * DARR_REALLOC_MULT);
    darr->data = realloc(darr->data, darr->available);
  }
}

void darr_append_byte(darr_t *darr, byte byte)
{
  darr_ensure_capacity(darr, 1);
  darr->data[darr->used++] = byte;
}

void darr_append_bytes(darr_t *darr, byte *bytes, size_t n)
{
  darr_ensure_capacity(darr, n);
  memcpy(darr->data + darr->used, bytes, n);
  darr->used += n;
}

byte darr_at(darr_t *darr, size_t index)
{
  if (index >= darr->used)
    // TODO: Error (index is out of bounds)
    return 0;
  return darr->data[index];
}
