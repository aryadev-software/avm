/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-23
 * Author: Aryadev Chavali
 * Description: Assembly source code compiler, targeting OVM
 */

#include <stdio.h>

#include <lib/darr.h>

typedef enum TokenType
{
  TOKEN_WHITESPACE,
  TOKEN_BYTE_LITERAL,
  TOKEN_HWORD_LITERAL,
  TOKEN_WORD_LITERAL,
  TOKEN_SYMBOL,
} token_type_t;

typedef struct
{
  token_type_t type;
  char *str;
  size_t str_size;
} token_t;

// We can use darr_read_file as an in memory buffer of source code
// Then just create new darr's for tokens, then instructions.  Then
// emit bytecode in the end.

int main(void)
{
  FILE *fp    = fopen("main.asm", "rb");
  darr_t darr = darr_read_file(fp);
  fclose(fp);
  darr.data[darr.available - 1] = '\0';
  printf("%lu/%lu\n%s\n", darr.used, darr.available, darr.data);
  free(darr.data);
  return 0;
}
