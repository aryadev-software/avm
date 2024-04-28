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
  return WORD_NTH_BYTE(w, 3) | (WORD_NTH_BYTE(w, 2) << 8) |
         (WORD_NTH_BYTE(w, 1) << 16) | (WORD_NTH_BYTE(w, 0) << 24);
}

word_t word_byteswap(const word_t w)
{
  return ((word_t)(hword_byteswap(WORD_NTH_HWORD(w, 0))) << 32) |
         hword_byteswap(WORD_NTH_HWORD(w, 1));
}

hword_t convert_bytes_to_hword(const byte_t *bytes)
{
  hword_t h = 0;
  if (LITTLE_ENDIAN)
    memcpy(&h, bytes, HWORD_SIZE);
  else
    for (size_t i = 0; i < HWORD_SIZE; ++i)
      h |= ((hword_t)(bytes[i]) << (8 * i));
  return h;
}

word_t convert_bytes_to_word(const byte_t *bytes)
{
  word_t h = 0;
  if (LITTLE_ENDIAN)
    memcpy(&h, bytes, WORD_SIZE);
  else
    for (size_t i = 0; i < WORD_SIZE; ++i)
      h |= ((word_t)(bytes[i]) << (8 * i));
  return h;
}

void convert_hword_to_bytes(hword_t w, byte_t *bytes)
{
  if (LITTLE_ENDIAN)
    memcpy(bytes, &w, HWORD_SIZE);
  else
    for (size_t i = 0; i < HWORD_SIZE; ++i)
      bytes[i] = WORD_NTH_BYTE(w, i);
}

void convert_word_to_bytes(word_t w, byte_t *bytes)
{
  if (LITTLE_ENDIAN)
    memcpy(bytes, &w, WORD_SIZE);
  else
    for (size_t i = 0; i < WORD_SIZE; ++i)
      bytes[i] = WORD_NTH_BYTE(w, i);
}
