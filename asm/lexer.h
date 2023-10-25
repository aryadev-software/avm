/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the GPLv2
 * license.  You should have received a copy of the GPLv2 license with
 * this file.  If not, please write to: aryadev@aryadevchavali.com.

 * Created: 2023-10-24
 * Author: Aryadev Chavali
 * Description: Lexer for assembly language
 */

#ifndef LEXER_H
#define LEXER_H

#include <lib/darr.h>

typedef enum TokenType
{
  TOKEN_LITERAL_BYTE,
  TOKEN_LITERAL_CHAR,
  TOKEN_LITERAL_HWORD,
  TOKEN_LITERAL_WORD,
  TOKEN_SYMBOL,
} token_type_t;

typedef struct
{
  token_type_t type;
  char *str;
  size_t str_size;
} token_t;

typedef darr_t buffer_t;

const char *token_type_as_cstr(token_type_t type);

token_t *tokenise_buffer(buffer_t *, size_t *);

#endif
