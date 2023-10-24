/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-23
 * Author: Aryadev Chavali
 * Description: Assembly source code compiler, targeting OVM
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <lib/darr.h>

typedef enum TokenType
{
  TOKEN_BYTE_LITERAL,
  TOKEN_HWORD_LITERAL,
  TOKEN_WORD_LITERAL,
  TOKEN_SYMBOL,
} token_type_t;

const char *token_type_as_cstr(token_type_t type)
{
  switch (type)
  {
  case TOKEN_BYTE_LITERAL:
    return "BYTE_LITERAL";
  case TOKEN_HWORD_LITERAL:
    return "HWORD_LITERAL";
  case TOKEN_WORD_LITERAL:
    return "WORD_LITERAL";
  case TOKEN_SYMBOL:
    return "SYMBOL";
  }
  return "";
}

typedef struct
{
  token_type_t type;
  char *str;
  size_t str_size;
} token_t;

// We can use darr_read_file as an in memory buffer of source code
// Then just create new darr's for tokens, then instructions.  Then
// emit bytecode in the end.
typedef darr_t buffer_t;

size_t space_left(buffer_t *buffer)
{
  if (buffer->available == buffer->used)
    return 0;
  return buffer->available - 1 - buffer->used;
}

bool is_symbol(char c)
{
  return isalpha(c) || c == '-' || c == '_';
}

token_t tokenise_symbol(buffer_t *buffer)
{
  token_t token = {.type = TOKEN_SYMBOL, .str_size = 0};
  for (; token.str_size < space_left(buffer) &&
         is_symbol(buffer->data[buffer->used + token.str_size]);
       ++token.str_size)
    continue;
  token.str = calloc(token.str_size + 1, 1);
  memcpy(token.str, buffer->data + buffer->used, token.str_size);
  token.str[token.str_size] = '\0';
  buffer->used += token.str_size;
  return token;
}

void tokenise_literal(buffer_t *buffer, token_t *token)
{
  token->str_size = 0;
  for (; token->str_size < space_left(buffer) &&
         isdigit(buffer->data[buffer->used + token->str_size]);
       ++token->str_size)
    continue;
  token->str = calloc(token->str_size + 1, 1);
  memcpy(token->str, buffer->data + buffer->used, token->str_size);
  token->str[token->str_size] = '\0';
  buffer->used += token->str_size;
}

token_t tokenise_byte_literal(buffer_t *buffer)
{
  token_t token = {.type = TOKEN_BYTE_LITERAL};
  tokenise_literal(buffer, &token);
  return token;
}

token_t tokenise_hword_literal(buffer_t *buffer)
{
  token_t token = {.type = TOKEN_HWORD_LITERAL};
  tokenise_literal(buffer, &token);
  return token;
}

token_t tokenise_word_literal(buffer_t *buffer)
{
  token_t token = {.type = TOKEN_WORD_LITERAL};
  tokenise_literal(buffer, &token);
  return token;
}

token_t *tokenise_buffer(buffer_t *buffer, size_t *n_tokens)
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
    else if (space_left(buffer) > 1 && isdigit(buffer->data[buffer->used + 1]))
    {
      // Parsing literals

      switch (c)
      {
      case 'b':
        buffer->used++;
        t = tokenise_byte_literal(buffer);
        break;
      case 'h':
        buffer->used++;
        t = tokenise_hword_literal(buffer);
        break;
      case 'w':
        buffer->used++;
        t = tokenise_word_literal(buffer);
        break;
      default:
        // TODO: Lex Error (INVALID_LITERAL)
        fprintf(stderr, "[LEX_ERROR]: Invalid literal `%c`\n", c);
        exit(1);
      }
    }
    else if (is_symbol(c))
    {
      t = tokenise_symbol(buffer);
    }
    if (is_token)
      darr_append_bytes(&tokens, (byte *)&t, sizeof(t));
  }
  *n_tokens = tokens.used / sizeof(token_t);
  return (token_t *)tokens.data;
}

int main(void)
{
  FILE *fp        = fopen("main.asm", "rb");
  buffer_t buffer = darr_read_file(fp);
  fclose(fp);

  size_t n        = 0;
  token_t *tokens = tokenise_buffer(&buffer, &n);
  printf("%lu bytes -> %lu tokens\n", buffer.used, n);
  free(buffer.data);

  for (size_t i = 0; i < n; ++i)
    printf("%s(%.*s)\n", token_type_as_cstr(tokens[i].type),
           (int)tokens[i].str_size, tokens[i].str);

  // Free the tokens
  for (size_t i = 0; i < n; ++i)
    free(tokens[i].str);
  free(tokens);
  return 0;
}
