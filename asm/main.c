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
#include "./parser.h"

int main(void)
{
  FILE *fp      = fopen("main.asm", "rb");
  darr_t buffer = darr_read_file(fp);
  fclose(fp);

  token_stream_t tokens = tokenise_buffer(&buffer);
  printf("%lu bytes -> %lu tokens\n", buffer.used, tokens.available);
  free(buffer.data);

  for (size_t i = 0; i < tokens.available; ++i)
    printf("%s(%.*s)\n",
           token_type_as_cstr((TOKEN_STREAM_AT(tokens.data, i)).type),
           (int)(TOKEN_STREAM_AT(tokens.data, i).str_size),
           (TOKEN_STREAM_AT(tokens.data, i).str));
  puts("");

  size_t number        = 0;
  inst_t *instructions = parse_stream(&tokens, &number);
  for (size_t i = 0; i < number; ++i)
  {
    inst_print(instructions[i], stdout);
    puts("");
  }
  // Free the tokens
  for (size_t i = 0; i < tokens.available; ++i)
    free(TOKEN_STREAM_AT(tokens.data, i).str);
  free(tokens.data);
  if (instructions)
    free(instructions);
  return 0;
}
