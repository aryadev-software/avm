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

hword convert_bytes_to_hword(byte *bytes)
{
  hword be_h = 0;
  memcpy(&be_h, bytes, HWORD_SIZE);
  hword h = be32toh(be_h);
  return h;
}

void convert_hword_to_bytes(hword w, byte *bytes)
{
  hword be_h = htobe32(w);
  memcpy(bytes, &be_h, HWORD_SIZE);
}

void convert_word_to_bytes(word w, byte *bytes)
{
  word be_w = htobe64(w);
  memcpy(bytes, &be_w, WORD_SIZE);
}

word convert_bytes_to_word(byte *bytes)
{
  word be_w = 0;
  memcpy(&be_w, bytes, WORD_SIZE);
  word w = be64toh(be_w);
  return w;
}
