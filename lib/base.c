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

union hword_pun
{
  hword_t h;
  byte_t bytes[HWORD_SIZE];
};

union word_pun
{
  word_t h;
  byte_t bytes[WORD_SIZE];
};

hword_t hword_htobc(hword_t w)
{
#if __LITTLE_ENDIAN__
  return w;
#else
  union hword_pun x = {w};
  union hword_pun y = {0};
  for (size_t i = 0, j = HWORD_SIZE; i < HWORD_SIZE; ++i, --j)
    y.bytes[j - 1] = x.bytes[i];
  return y.h;
#endif
}

hword_t hword_bctoh(hword_t w)
{
#if __LITTLE_ENDIAN__
  return w;
#else
  union hword_pun x = {w};
  union hword_pun y = {0};
  for (size_t i = 0, j = HWORD_SIZE; i < HWORD_SIZE; ++i, --j)
    y.bytes[j - 1] = x.bytes[i];
  return y.h;
#endif
}

word_t word_htobc(word_t w)
{
#if __LITTLE_ENDIAN__
  return w;
#else
  union word_pun x = {w};
  union word_pun y = {0};
  for (size_t i = 0, j = WORD_SIZE; i < WORD_SIZE; ++i, --j)
    y.bytes[j - 1] = x.bytes[i];
  return y.h;
#endif
}

word_t word_bctoh(word_t w)
{
#if __LITTLE_ENDIAN__
  return w;
#else
  union word_pun x = {w};
  union word_pun y = {0};
  for (size_t i = 0, j = WORD_SIZE; i < WORD_SIZE; ++i, --j)
    y.bytes[j - 1] = x.bytes[i];
  return y.h;
#endif
}

hword_t convert_bytes_to_hword(byte_t *bytes)
{
  hword_t be_h = 0;
  memcpy(&be_h, bytes, HWORD_SIZE);
  hword_t h = hword_bctoh(be_h);
  return h;
}

void convert_hword_to_bytes(hword_t w, byte_t *bytes)
{
  hword_t be_h = hword_htobc(w);
  memcpy(bytes, &be_h, HWORD_SIZE);
}

void convert_word_to_bytes(word_t w, byte_t *bytes)
{
  word_t be_w = word_htobc(w);
  memcpy(bytes, &be_w, WORD_SIZE);
}

word_t convert_bytes_to_word(byte_t *bytes)
{
  word_t be_w = 0;
  memcpy(&be_w, bytes, WORD_SIZE);
  word_t w = word_bctoh(be_w);
  return w;
}
