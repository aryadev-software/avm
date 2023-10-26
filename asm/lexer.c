/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-24
 * Author: Aryadev Chavali
 * Description: Lexer for assembly language
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "./lexer.h"

const char *token_type_as_cstr(token_type_t type)
{
  switch (type)
  {
  case TOKEN_LITERAL_NUMBER:
    return "LITERAL_NUMBER";
  case TOKEN_LITERAL_CHAR:
    return "LITERAL_CHAR";
  case TOKEN_SYMBOL:
    return "SYMBOL";
  }
  return "";
}

size_t space_left(buffer_t *buffer)
{
  if (buffer->available == buffer->used)
    return 0;
  return buffer->available - 1 - buffer->used;
}

bool is_symbol(char c)
{
  return isalpha(c) || c == '-' || c == '_' || c == '.';
}

char uppercase(char c)
{
  if (c >= 'a' && c <= 'z')
    return (c - 'a') + 'A';
  return c;
}

token_t tokenise_symbol(buffer_t *buffer)
{
  token_t token = {.type = TOKEN_SYMBOL, .str_size = 0};
  for (; token.str_size < space_left(buffer) &&
         is_symbol(buffer->data[buffer->used + token.str_size]);
       ++token.str_size)
    buffer->data[buffer->used + token.str_size] =
        uppercase(buffer->data[buffer->used + token.str_size]);
  token.str = calloc(token.str_size + 1, 1);
  memcpy(token.str, buffer->data + buffer->used, token.str_size);
  token.str[token.str_size] = '\0';
  buffer->used += token.str_size;
  return token;
}

token_t tokenise_number_literal(buffer_t *buffer)
{
  token_t token = {.type = TOKEN_LITERAL_NUMBER, .str_size = 0};
  if (buffer->data[buffer->used] == '-')
    ++token.str_size;
  for (; token.str_size < space_left(buffer) &&
         isdigit(buffer->data[buffer->used + token.str_size]);
       ++token.str_size)
    continue;
  token.str = calloc(token.str_size + 1, 1);
  memcpy(token.str, buffer->data + buffer->used, token.str_size);
  token.str[token.str_size] = '\0';
  buffer->used += token.str_size;
  return token;
}

token_t tokenise_char_literal(buffer_t *buffer)
{
  token_t token = {.type = TOKEN_LITERAL_CHAR, .str_size = 1};
  token.str     = calloc(1, 1);
  token.str[0]  = buffer->data[buffer->used + 1];
  buffer->used += 3;
  return token;
}

token_stream_t tokenise_buffer(buffer_t *buffer)
{
  buffer_t tokens = {0};
  darr_init(&tokens, sizeof(token_t));
  while (space_left(buffer) != 0)
  {
    bool is_token = true;
    token_t t     = {0};
    char c        = buffer->data[buffer->used];
    if (isspace(c) || c == '\0')
    {
      // Clean whitespace
      for (; space_left(buffer) > 0 && (isspace(c) || c == '\0');
           ++buffer->used, c = buffer->data[buffer->used])
        continue;
      is_token = false;
    }
    else if (isdigit(c) || (space_left(buffer) > 1 && c == '-' &&
                            isdigit(buffer->data[buffer->used + 1])))
      t = tokenise_number_literal(buffer);
    else if (is_symbol(c))
      t = tokenise_symbol(buffer);
    else if (c == '\'')
    {
      if (space_left(buffer) < 2 || buffer->data[buffer->used + 2] != '\'')
        // TODO: Lex Error (INVALID_CHAR_LITERAL)
        exit(1);
      t = tokenise_char_literal(buffer);
    }

    if (is_token)
      darr_append_bytes(&tokens, (byte *)&t, sizeof(t));
  }
  size_t n_tokens  = tokens.used / sizeof(token_t);
  tokens.available = n_tokens;
  tokens.used      = 0;
  return tokens;
}
