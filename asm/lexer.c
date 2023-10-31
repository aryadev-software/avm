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

const char *lerr_as_cstr(lerr_t lerr)
{
  switch (lerr)
  {
  case LERR_INVALID_CHAR_LITERAL:
    return "INVALID_CHAR_LITERAL";
    break;
  case LERR_OK:
    return "OK";
    break;
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

token_t tokenise_symbol(buffer_t *buffer, size_t *column)
{
  token_t token = {.type = TOKEN_SYMBOL, .str_size = 0, .column = *column};
  for (; token.str_size < space_left(buffer) &&
         is_symbol(buffer->data[buffer->used + token.str_size]);
       ++token.str_size)
    buffer->data[buffer->used + token.str_size] =
        uppercase(buffer->data[buffer->used + token.str_size]);
  token.str = calloc(token.str_size + 1, 1);
  memcpy(token.str, buffer->data + buffer->used, token.str_size);
  token.str[token.str_size] = '\0';
  buffer->used += token.str_size;
  *column += token.str_size;
  return token;
}

token_t tokenise_number_literal(buffer_t *buffer, size_t *column)
{
  token_t token = {
      .type = TOKEN_LITERAL_NUMBER, .str_size = 0, .column = *column};
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
  *column += token.str_size;
  return token;
}

bool is_valid_hex_char(char c)
{
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

token_t tokenise_hex_literal(buffer_t *buffer, size_t *column)
{
  // For the x part of the literal
  ++buffer->used;
  token_t token = {
      .type = TOKEN_LITERAL_NUMBER, .str_size = 0, .column = *column};
  for (; token.str_size < space_left(buffer) &&
         is_valid_hex_char(buffer->data[buffer->used + token.str_size]);
       ++token.str_size)
    continue;
  // Setup a proper C hex literal
  token.str    = calloc(token.str_size + 3, 1);
  token.str[0] = '0';
  token.str[1] = 'x';
  memcpy(token.str + 2, buffer->data + buffer->used, token.str_size);
  token.str[token.str_size + 2] = '\0';
  buffer->used += token.str_size;
  *column += token.str_size;

  // Setup the first two characters
  token.str_size += 2;
  printf("hex_literal: %s, %lu\n", token.str, token.str_size);
  return token;
}

token_t tokenise_char_literal(buffer_t *buffer, size_t *column)
{
  token_t token = {
      .type = TOKEN_LITERAL_CHAR, .str_size = 1, .column = *column};
  token.str    = calloc(1, 1);
  token.str[0] = buffer->data[buffer->used + 1];
  buffer->used += 3;
  *column += 3;
  return token;
}

lerr_t tokenise_buffer(buffer_t *buffer, token_stream_t *tokens_ptr)
{
  size_t column = 0, line = 1;
  token_stream_t tokens = {0};
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
      {
        ++column;
        if (c == '\n')
        {
          column = 0;
          ++line;
        }
      }
      ++column;
      is_token = false;
    }
    else if (c == ';')
    {
      // Start lexing at next line
      for (; space_left(buffer) > 0 && c != '\n';
           ++buffer->used, c = buffer->data[buffer->used])
        continue;
      column = 0;
      ++line;
      ++buffer->used;
      is_token = false;
    }
    else if (isdigit(c) || (space_left(buffer) > 1 && c == '-' &&
                            isdigit(buffer->data[buffer->used + 1])))
      t = tokenise_number_literal(buffer, &column);
    else if (c == 'x' && space_left(buffer) > 1 &&
             is_valid_hex_char(buffer->data[buffer->used + 1]))
      t = tokenise_hex_literal(buffer, &column);
    else if (is_symbol(c))
      t = tokenise_symbol(buffer, &column);
    else if (c == '\'')
    {
      if (space_left(buffer) < 2)
      {
        free(tokens.data);
        return LERR_INVALID_CHAR_LITERAL;
      }
      else if (buffer->data[buffer->used + 1] == '\\')
      {
        char escape = '\0';
        if (space_left(buffer) < 3 || buffer->data[buffer->used + 3] != '\'')
        {
          free(tokens.data);
          return LERR_INVALID_CHAR_LITERAL;
        }
        switch (buffer->data[buffer->used + 2])
        {
        case 'n':
          escape = '\n';
          break;
        case 't':
          escape = '\t';
          break;
        case 'r':
          escape = '\r';
          break;
        case '\\':
          escape = '\\';
          break;
        default:
          column += 2;
          free(tokens.data);
          return LERR_INVALID_CHAR_LITERAL;
          break;
        }

        t = (token_t){.type     = TOKEN_LITERAL_CHAR,
                      .str      = malloc(1),
                      .str_size = 1,
                      .column   = column};
        column += 4;
        buffer->used += 4;
        t.str[0] = escape;
      }
      else
        t = tokenise_char_literal(buffer, &column);
    }

    if (is_token)
    {
      t.line = line;
      darr_append_bytes(&tokens, (byte *)&t, sizeof(t));
    }
  }
  size_t n_tokens  = tokens.used / sizeof(token_t);
  tokens.available = n_tokens;
  tokens.used      = 0;
  *tokens_ptr      = tokens;
  return LERR_OK;
}
