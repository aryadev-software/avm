/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-26
 * Author: Aryadev Chavali
 * Description: Implementation of basic library functions
 */

#include "./base.h"

#include <string.h>

hword_t hword_byteswap(const hword_t w)
{
  // TODO: is there a faster way of doing this?
  return WORD_NTH_BYTE(w, 3) | (WORD_NTH_BYTE(w, 2) << 8) |
         WORD_NTH_BYTE(w, 1) << 16 | WORD_NTH_BYTE(w, 0) << 24;
}

word_t word_byteswap(const word_t w)
{
  // TODO: is there a faster way of doing this?
  return WORD_NTH_BYTE(w, 7) | WORD_NTH_BYTE(w, 6) << 8 |
         WORD_NTH_BYTE(w, 5) << 16 | WORD_NTH_BYTE(w, 4) << 24 |
         WORD_NTH_BYTE(w, 3) << 32 | WORD_NTH_BYTE(w, 2) << 40 |
         WORD_NTH_BYTE(w, 1) << 48 | WORD_NTH_BYTE(w, 0) << 56;
}

hword_t convert_bytes_to_hword(const byte_t *bytes)
{
  hword_t h = 0;
  memcpy(&h, bytes, HWORD_SIZE);
  if (!LITTLE_ENDIAN)
    h = hword_byteswap(h);
  return h;
}

word_t convert_bytes_to_word(const byte_t *bytes)
{
  word_t h = 0;
  memcpy(&h, bytes, WORD_SIZE);
  if (!LITTLE_ENDIAN)
    h = word_byteswap(h);
  return h;
}

void convert_hword_to_bytes(hword_t w, byte_t *bytes)
{
  hword_t h = LITTLE_ENDIAN ? w : hword_byteswap(w);
  memcpy(bytes, &h, HWORD_SIZE);
}

void convert_word_to_bytes(word_t w, byte_t *bytes)
{
  word_t h = LITTLE_ENDIAN ? w : word_byteswap(w);
  memcpy(bytes, &h, WORD_SIZE);
}
