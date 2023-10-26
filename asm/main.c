/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-23
 * Author: Aryadev Chavali
 * Description: Assembly source code compiler, targeting OVM
 */

#include <lib/darr.h>

#include "./lexer.h"

int main(void)
{
  FILE *fp      = fopen("main.asm", "rb");
  darr_t buffer = darr_read_file(fp);
  fclose(fp);

  token_stream_t tokens = tokenise_buffer(&buffer);
  printf("%lu bytes -> %lu tokens\n", buffer.used, tokens.available);
  free(buffer.data);

  for (size_t i = 0; i < tokens.available; ++i)
    printf("%s(%.*s)\n", token_type_as_cstr(((token_t *)tokens.data)[i].type),
           (int)((token_t *)tokens.data)[i].str_size,
           ((token_t *)tokens.data)[i].str);

  // Free the tokens
  for (size_t i = 0; i < tokens.available; ++i)
    free(((token_t *)tokens.data)[i].str);
  free(tokens.data);
  return 0;
}
