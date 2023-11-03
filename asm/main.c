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

void usage(const char *program_name, FILE *fp)
{
  fprintf(fp,
          "Usage: %s FILE OUT-FILE\n"
          "\tFILE: Source code to compile\n"
          "\tOUT-FILE: Name of file to store bytecode\n",
          program_name);
}

int main(int argc, char *argv[])
{
  int ret           = 0;
  char *source_file = "";
  char *out_file    = "";
  if (argc < 3)
  {
    usage(argv[0], stderr);
    return 1;
  }

  source_file = argv[1];
  out_file    = argv[2];

#if VERBOSE >= 1
  printf("[%sASSEMBLER%s]: Assembling `%s` to `%s`\n", TERM_YELLOW, TERM_RESET,
         source_file, out_file);
#endif
  FILE *fp      = fopen(source_file, "rb");
  darr_t buffer = darr_read_file(fp);
  fclose(fp);

  token_stream_t tokens = {0};
  lerr_t lex_error      = tokenise_buffer(&buffer, &tokens);
  if (lex_error)
  {
    // Compute the line/newlines by hand
    size_t column = 0, line = 1;
    for (size_t i = 0; i < buffer.used; ++i)
    {
      if (buffer.data[i] == '\n')
      {
        column = 0;
        ++line;
      }
      else
        ++column;
    }
    fprintf(stderr, "%s:%lu:%lu: %s\n", source_file, line, column,
            lerr_as_cstr(lex_error));
    ret = 255 - lex_error;
    goto end;
  }
#if VERBOSE >= 1
  printf("[%sTOKENISER%s]: %lu bytes -> %lu tokens\n", TERM_GREEN, TERM_RESET,
         buffer.used, tokens.available);
#endif

#if VERBOSE >= 2
  printf("[%sTOKENISER%s]: Tokens parsed:\n", TERM_GREEN, TERM_RESET);
  for (size_t i = 0; i < tokens.available; ++i)
  {
    token_t token = TOKEN_STREAM_AT(tokens.data, i);
    printf("\t[%lu]: %s(`%s`)@%lu,%lu\n", i, token_type_as_cstr(token.type),
           token.str, token.line, token.column);
  }
#endif

  free(buffer.data);
  buffer.data = NULL;

#if VERBOSE >= 2
  printf("[%sPARSER%s]: Beginning parse...\n", TERM_YELLOW, TERM_RESET);
#endif
  prog_t *program    = NULL;
  perr_t parse_error = parse_stream(&tokens, &program);
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
    fprintf(stderr, "%s:%lu:%lu: %s\n", source_file, line, column,
            perr_as_cstr(parse_error));
    ret = 255 - parse_error;
    goto end;
  }
#if VERBOSE >= 1
  printf("[%sPARSER%s]: %lu tokens -> %lu instructions\n", TERM_GREEN,
         TERM_RESET, tokens.available, program->count);
#endif

#if VERBOSE >= 2
  printf("[%sPARSER%s]: Instructions parsed:\n", TERM_GREEN, TERM_RESET);
  for (size_t i = 0; i < program->count; ++i)
  {
    printf("\t[%lu]: ", i);
    inst_print(program->instructions[i], stdout);
    printf("\n");
  }
#endif

  fp = fopen(out_file, "wb");
  prog_write_file(program, fp);
  fclose(fp);
#if VERBOSE >= 1
  printf("[%sASSEMBLER%s]: Wrote bytecode to `%s`\n", TERM_GREEN, TERM_RESET,
         out_file);
#endif
end:
  if (buffer.data)
    free(buffer.data);
  if (tokens.data)
  {
    for (size_t i = 0; i < tokens.available; ++i)
      free(TOKEN_STREAM_AT(tokens.data, i).str);
    free(tokens.data);
  }
  if (program)
    free(program);
  return ret;
}
