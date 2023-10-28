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
  int ret              = 0;
  const char *filename = "main.asm";
  FILE *fp             = fopen(filename, "rb");
  darr_t buffer        = darr_read_file(fp);
  fclose(fp);

  token_stream_t tokens = tokenise_buffer(&buffer);
  printf("%lu bytes -> %lu tokens\n", buffer.used, tokens.available);
  free(buffer.data);

  size_t number        = 0;
  inst_t *instructions = NULL;
  perr_t parse_error   = parse_stream(&tokens, &instructions, &number);
  if (parse_error)
  {
    size_t column = 0;
    size_t line   = 0;
    if (tokens.used < tokens.available)
    {
      token_t t = TOKEN_STREAM_AT(tokens.data, tokens.used);
      column    = t.column;
      line      = t.line;
    }
    fprintf(stderr, "%s:%lu:%lu: %s\n", filename, line, column,
            perr_as_cstr(parse_error));
    ret = 255 - parse_error;
    goto end;
  }
  for (size_t i = 0; i < number; ++i)
  {
    inst_print(instructions[i], stdout);
    puts("");
  }
  // Free the tokens
end:
  if (tokens.data)
  {
    for (size_t i = 0; i < tokens.available; ++i)
      free(TOKEN_STREAM_AT(tokens.data, i).str);
    free(tokens.data);
  }
  if (instructions)
    free(instructions);
  return ret;
}
