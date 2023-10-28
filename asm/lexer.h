/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-24
 * Author: Aryadev Chavali
 * Description: Lexer for assembly language
 */

#ifndef LEXER_H
#define LEXER_H

#include <lib/darr.h>

typedef enum TokenType
{
  TOKEN_LITERAL_NUMBER,
  TOKEN_LITERAL_CHAR,
  TOKEN_SYMBOL,
} token_type_t;

typedef struct
{
  token_type_t type;
  size_t column, line;
  char *str;
  size_t str_size;
} token_t;

typedef darr_t buffer_t;
typedef darr_t token_stream_t;
#define TOKEN_STREAM_AT(STREAM_DATA, INDEX) (((token_t *)(STREAM_DATA))[INDEX])

const char *token_type_as_cstr(token_type_t type);
token_stream_t tokenise_buffer(buffer_t *);

#endif
