/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-26
 * Author: Aryadev Chavali
 * Description: Implementation of basic library functions
 */

#include <string.h>

#include "./base.h"

hword convert_bytes_to_hword(byte *bytes)
{
  hword h = 0;
  memcpy(&h, bytes, HWORD_SIZE);
  return h;
}

void convert_hword_to_bytes(hword w, byte *bytes)
{
  memcpy(bytes, &w, HWORD_SIZE);
}

void convert_word_to_bytes(word w, byte *bytes)
{
  memcpy(bytes, &w, WORD_SIZE);
}

word convert_bytes_to_word(byte *bytes)
{
  word w = 0;
  memcpy(&w, bytes, WORD_SIZE);
  return w;
}
