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
  TOKEN_NOOP,
  TOKEN_HALT,
  TOKEN_PUSH,
  TOKEN_POP,
  TOKEN_PUSH_REG,
  TOKEN_MOV,
  TOKEN_DUP,
  TOKEN_MALLOC,
  TOKEN_MSET,
  TOKEN_MSET_STACK,
  TOKEN_MGET,
  TOKEN_MGET_STACK,
  TOKEN_MDELETE,
  TOKEN_MSIZE,
  TOKEN_NOT,
  TOKEN_OR,
  TOKEN_AND,
  TOKEN_XOR,
  TOKEN_EQ,
  TOKEN_LT,
  TOKEN_LTE,
  TOKEN_GT,
  TOKEN_GTE,
  TOKEN_PLUS,
  TOKEN_MULT,
  TOKEN_PRINT,
  TOKEN_JUMP,
  TOKEN_JUMP_IF,
  TOKEN_SYMBOL,
} token_type_t;

typedef struct
{
  token_type_t type;
  size_t column, line;
  char *str;
  size_t str_size;
} token_t;

typedef enum
{
  LERR_OK = 0,
  LERR_INVALID_CHAR_LITERAL,
} lerr_t;
const char *lerr_as_cstr(lerr_t);

typedef darr_t buffer_t;
typedef darr_t token_stream_t;
#define TOKEN_STREAM_AT(STREAM_DATA, INDEX) (((token_t *)(STREAM_DATA))[INDEX])

const char *token_type_as_cstr(token_type_t type);
lerr_t tokenise_buffer(buffer_t *, token_stream_t *);

#endif
