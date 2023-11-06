/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-24
 * Author: Aryadev Chavali
 * Description: Lexer for assembly language
 */

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <lib/inst.h>

#include "./lexer.h"

const char *token_type_as_cstr(token_type_t type)
{
  switch (type)
  {
  case TOKEN_PP_CONST:
    return "PP_CONST";
  case TOKEN_PP_END:
    return "PP_END";
  case TOKEN_PP_REFERENCE:
    return "PP_REFERENCE";
  case TOKEN_GLOBAL:
    return "GLOBAL";
  case TOKEN_STAR:
    return "STAR";
  case TOKEN_LITERAL_NUMBER:
    return "LITERAL_NUMBER";
  case TOKEN_LITERAL_CHAR:
    return "LITERAL_CHAR";
  case TOKEN_NOOP:
    return "NOOP";
  case TOKEN_HALT:
    return "HALT";
  case TOKEN_PUSH:
    return "PUSH";
  case TOKEN_POP:
    return "POP";
  case TOKEN_PUSH_REG:
    return "PUSH_REG";
  case TOKEN_MOV:
    return "MOV";
  case TOKEN_DUP:
    return "DUP";
  case TOKEN_MALLOC:
    return "MALLOC";
  case TOKEN_MALLOC_STACK:
    return "MALLOC_STACK";
  case TOKEN_MSET:
    return "MSET";
  case TOKEN_MSET_STACK:
    return "MSET_STACK";
  case TOKEN_MGET:
    return "MGET";
  case TOKEN_MGET_STACK:
    return "MGET_STACK";
  case TOKEN_MDELETE:
    return "MDELETE";
  case TOKEN_MSIZE:
    return "MSIZE";
  case TOKEN_NOT:
    return "NOT";
  case TOKEN_OR:
    return "OR";
  case TOKEN_AND:
    return "AND";
  case TOKEN_XOR:
    return "XOR";
  case TOKEN_EQ:
    return "EQ";
  case TOKEN_LT:
    return "LT";
  case TOKEN_LTE:
    return "LTE";
  case TOKEN_GT:
    return "GT";
  case TOKEN_GTE:
    return "GTE";
  case TOKEN_PLUS:
    return "PLUS";
  case TOKEN_SUB:
    return "SUB";
  case TOKEN_MULT:
    return "MULT";
  case TOKEN_PRINT:
    return "PRINT";
  case TOKEN_JUMP_ABS:
    return "JUMP_ABS";
  case TOKEN_JUMP_STACK:
    return "JUMP_STACK";
  case TOKEN_JUMP_IF:
    return "JUMP_IF";
  case TOKEN_CALL:
    return "CALL";
  case TOKEN_CALL_STACK:
    return "CALL_STACK";
  case TOKEN_RET:
    return "RET";
  case TOKEN_SYMBOL:
    return "SYMBOL";
  }
  return "";
}

const char *lerr_as_cstr(lerr_t lerr)
{
  switch (lerr)
  {
  case LERR_OK:
    return "OK";
  case LERR_INVALID_CHAR_LITERAL:
    return "INVALID_CHAR_LITERAL";
  case LERR_INVALID_PREPROCESSOR_DIRECTIVE:
    return "INVALID_PREPROCESSOR_DIRECTIVE";
  }
  return "";
}

size_t space_left(buffer_t *buffer)
{
  if (buffer->available == buffer->used)
    return 0;
  return buffer->available - 1 - buffer->used;
}

char uppercase(char c)
{
  if (c >= 'a' && c <= 'z')
    return (c - 'a') + 'A';
  return c;
}

bool is_symbol(char c)
{
  return isalpha(c) || isdigit(c) || c == '-' || c == '_' || c == '.' ||
         c == ':';
}

bool is_valid_hex_char(char c)
{
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

lerr_t tokenise_symbol(buffer_t *buffer, size_t *column, token_t *token)
{
  static_assert(NUMBER_OF_OPCODES == 98, "tokenise_buffer: Out of date!");

  size_t sym_size = 0;
  for (; sym_size < space_left(buffer) &&
         is_symbol(buffer->data[buffer->used + sym_size]);
       ++sym_size)
    buffer->data[buffer->used + sym_size] =
        uppercase(buffer->data[buffer->used + sym_size]);

  token_t ret  = {0};
  char *opcode = (char *)buffer->data + buffer->used;

  bool is_opcode    = true;
  token_type_t type = 0;
  size_t offset     = 0;

  if (sym_size > 1 && strncmp(opcode, "%", 1) == 0)
  {
    // Some preprocessing directive
    if (sym_size > 6 && strncmp(opcode + 1, "CONST", 5) == 0)
    {
      type   = TOKEN_PP_CONST;
      offset = 6;
    }
    else if (sym_size == 4 && strncmp(opcode + 1, "END", 3) == 0)
    {
      type   = TOKEN_PP_END;
      offset = 4;
    }
    else
      return LERR_INVALID_PREPROCESSOR_DIRECTIVE;
  }
  else if (sym_size > 1 && strncmp(opcode, "$", 1) == 0)
  {
    // A reference to a preprocessing constant
    offset = 1;
    type   = TOKEN_PP_REFERENCE;
  }
  else if (sym_size == 4 && strncmp(opcode, "NOOP", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_NOOP;
  }
  else if (sym_size == 4 && strncmp(opcode, "HALT", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_HALT;
  }
  else if (sym_size > 9 && strncmp(opcode, "PUSH.REG.", 9) == 0)
  {
    offset = 9;
    type   = TOKEN_PUSH_REG;
  }
  else if (sym_size > 5 && strncmp(opcode, "PUSH.", 5) == 0)
  {
    offset = 5;
    type   = TOKEN_PUSH;
  }
  else if (sym_size > 4 && strncmp(opcode, "POP.", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_POP;
  }
  else if (sym_size > 4 && strncmp(opcode, "MOV.", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_MOV;
  }
  else if (sym_size > 4 && strncmp(opcode, "DUP.", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_DUP;
  }
  else if (sym_size > 13 && strncmp(opcode, "MALLOC.STACK.", 13) == 0)
  {
    offset = 13;
    type   = TOKEN_MALLOC_STACK;
  }
  else if (sym_size > 7 && strncmp(opcode, "MALLOC.", 7) == 0)
  {
    offset = 7;
    type   = TOKEN_MALLOC;
  }
  else if (sym_size > 11 && strncmp(opcode, "MSET.STACK.", 11) == 0)
  {
    offset = 11;
    type   = TOKEN_MSET_STACK;
  }
  else if (sym_size > 5 && strncmp(opcode, "MSET.", 5) == 0)
  {
    offset = 5;
    type   = TOKEN_MSET;
  }
  else if (sym_size > 11 && strncmp(opcode, "MGET.STACK.", 11) == 0)
  {
    offset = 11;
    type   = TOKEN_MGET_STACK;
  }
  else if (sym_size > 5 && strncmp(opcode, "MGET.", 5) == 0)
  {
    offset = 5;
    type   = TOKEN_MGET;
  }
  else if (sym_size == 7 && strncmp(opcode, "MDELETE", 7) == 0)
  {
    offset = 7;
    type   = TOKEN_MDELETE;
  }
  else if (sym_size == 5 && strncmp(opcode, "MSIZE", 5) == 0)
  {
    offset = 5;
    type   = TOKEN_MSIZE;
  }
  else if (sym_size > 4 && strncmp(opcode, "NOT.", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_NOT;
  }
  else if (sym_size > 3 && strncmp(opcode, "OR.", 3) == 0)
  {
    offset = 3;
    type   = TOKEN_OR;
  }
  else if (sym_size > 4 && strncmp(opcode, "AND.", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_AND;
  }
  else if (sym_size > 4 && strncmp(opcode, "XOR.", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_XOR;
  }
  else if (sym_size >= 3 && strncmp(opcode, "EQ.", 3) == 0)
  {
    offset = 3;
    type   = TOKEN_EQ;
  }
  else if (sym_size > 4 && strncmp(opcode, "LTE.", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_LTE;
  }
  else if (sym_size > 3 && strncmp(opcode, "LT.", 3) == 0)
  {
    offset = 3;
    type   = TOKEN_LT;
  }
  else if (sym_size > 4 && strncmp(opcode, "GTE.", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_GTE;
  }
  else if (sym_size > 3 && strncmp(opcode, "GT.", 3) == 0)
  {
    offset = 3;
    type   = TOKEN_GT;
  }
  else if (sym_size > 4 && strncmp(opcode, "SUB.", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_SUB;
  }
  else if (sym_size > 5 && strncmp(opcode, "PLUS.", 5) == 0)
  {
    offset = 5;
    type   = TOKEN_PLUS;
  }
  else if (sym_size > 5 && strncmp(opcode, "MULT.", 5) == 0)
  {
    offset = 5;
    type   = TOKEN_MULT;
  }
  else if (sym_size > 6 && strncmp(opcode, "PRINT.", 6) == 0)
  {
    offset = 6;
    type   = TOKEN_PRINT;
  }
  else if (sym_size == 8 && strncmp(opcode, "JUMP.ABS", 8) == 0)
  {
    offset = 8;
    type   = TOKEN_JUMP_ABS;
  }
  else if (sym_size == 10 && strncmp(opcode, "JUMP.STACK", 10) == 0)
  {
    offset = 10;
    type   = TOKEN_JUMP_STACK;
  }
  else if (sym_size > 8 && strncmp(opcode, "JUMP.IF.", 8) == 0)
  {
    offset = 8;
    type   = TOKEN_JUMP_IF;
  }
  else if (sym_size == 10 && strncmp(opcode, "CALL.STACK", 10) == 0)
  {
    offset = 10;
    type   = TOKEN_CALL_STACK;
  }
  else if (sym_size == 4 && strncmp(opcode, "CALL", 4) == 0)
  {
    offset = 4;
    type   = TOKEN_CALL;
  }
  else if (sym_size == 3 && strncmp(opcode, "RET", 3) == 0)
  {
    offset = 3;
    type   = TOKEN_RET;
  }
  else if (sym_size == 6 && strncmp(opcode, "GLOBAL", 6) == 0)
  {
    offset = 6;
    type   = TOKEN_GLOBAL;
  }
  else
    is_opcode = false;

  if (!is_opcode)
  {
    // Just a symbol, so no further manipulation
    char *sym = malloc(sym_size + 1);
    memcpy(sym, opcode, sym_size);
    sym[sym_size] = '\0';
    ret           = (token_t){.type     = TOKEN_SYMBOL,
                              .str      = sym,
                              .column   = *column,
                              .str_size = sym_size};
  }
  else
  {
    ret.type   = type;
    ret.column = *column;
    if (offset == sym_size)
    {
      // There's no more to the string
      ret.str    = malloc(1);
      ret.str[0] = '\0';
    }
    else
    {
      // t.str is the remaining part of the string after the
      // opcode
      ret.str = calloc(sym_size - offset + 1, 1);
      memcpy(ret.str, opcode + offset, sym_size - offset);
      ret.str[sym_size - offset] = '\0';
    }
    ret.str_size = sym_size - offset;
  }
  *column += sym_size - 1;
  buffer->used += sym_size;
  *token = ret;
  return LERR_OK;
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
  return token;
}

token_t tokenise_char_literal(buffer_t *buffer, size_t *column)
{
  token_t token = {
      .type = TOKEN_LITERAL_CHAR, .str_size = 1, .column = *column};
  token.str    = calloc(2, 1);
  token.str[0] = buffer->data[buffer->used + 1];
  token.str[1] = '\0';
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
    else if (c == '*')
    {
      t        = (token_t){.type     = TOKEN_STAR,
                           .column   = column,
                           .str      = malloc(1),
                           .str_size = 1};
      t.str[0] = '\0';
      ++buffer->used;
    }
    else if (isdigit(c) || (space_left(buffer) > 1 && c == '-' &&
                            isdigit(buffer->data[buffer->used + 1])))
      t = tokenise_number_literal(buffer, &column);
    else if (c == 'x' && space_left(buffer) > 1 &&
             is_valid_hex_char(buffer->data[buffer->used + 1]))
      t = tokenise_hex_literal(buffer, &column);
    else if (is_symbol(c))
    {
      lerr_t lerr = tokenise_symbol(buffer, &column, &t);
      if (lerr)
      {
        free(tokens.data);
        return lerr;
      }
    }
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
                      .str      = malloc(2),
                      .str_size = 1,
                      .column   = column};
        column += 2;
        buffer->used += 4;
        t.str[0] = escape;
        t.str[1] = '\0';
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
