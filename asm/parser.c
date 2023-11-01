/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-24
 * Author: Aryadev Chavali
 * Description: Parser for assembly language
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "./parser.h"

#define OPCODE_ON_TYPE(BASE_CODE, TYPE)

const char *perr_as_cstr(perr_t perr)
{
  switch (perr)
  {
  case PERR_OK:
    return "OK";
  case PERR_INTEGER_OVERFLOW:
    return "INTEGER_OVERFLOW";
  case PERR_NOT_A_NUMBER:
    return "NOT_A_NUMBER";
  case PERR_EXPECTED_TYPE:
    return "EXPECTED_TYPE";
  case PERR_EXPECTED_UTYPE:
    return "EXPECTED_UTYPE";
  case PERR_EXPECTED_SYMBOL:
    return "EXPECTED_SYMBOL";
  case PERR_EXPECTED_OPERAND:
    return "EXPECTED_OPERAND";
  case PERR_UNKNOWN_OPERATOR:
    return "UNKNOWN_OPERATOR";
  default:
    return "";
  }
}

perr_t parse_word(token_t token, word *ret)
{
  if (token.type == TOKEN_LITERAL_NUMBER)
  {
    bool is_negative = token.str_size > 1 && token.str[0] == '-';
    word w           = 0;
    if (is_negative)
    {
      char *end = NULL;
      s_word i  = strtoll(token.str, &end, 0);
      if (!(end && end[0] == '\0'))
        return PERR_NOT_A_NUMBER;
      else if (errno == ERANGE)
      {
        errno = 0;
        return PERR_INTEGER_OVERFLOW;
      }
      // Copy bits, do not cast
      memcpy(&w, &i, sizeof(w));
    }
    else
    {
      char *end = NULL;
      w         = strtoull(token.str, &end, 0);
      if (!(end && end[0] == '\0'))
        return PERR_NOT_A_NUMBER;
      else if (errno == ERANGE)
      {
        errno = 0;
        return PERR_INTEGER_OVERFLOW;
      }
    }
    *ret = w;
    return PERR_OK;
  }
  else if (token.type == TOKEN_LITERAL_CHAR)
  {
    *ret = token.str[0];
    return PERR_OK;
  }
  else
    return PERR_NOT_A_NUMBER;
}

enum Type
{
  T_NIL = -1,
  T_BYTE,
  T_CHAR,
  T_HWORD,
  T_INT,
  T_LONG,
  T_WORD,
} parse_details_to_type(token_t details)
{
  if (details.str_size == 5 && strncmp(details.str, ".BYTE", 5) == 0)
    return T_BYTE;
  else if (details.str_size == 5 && strncmp(details.str, ".CHAR", 5) == 0)
    return T_CHAR;
  else if (details.str_size == 6 && strncmp(details.str, ".HWORD", 6) == 0)
    return T_HWORD;
  else if (details.str_size == 4 && strncmp(details.str, ".INT", 4) == 0)
    return T_INT;
  else if (details.str_size == 5 && strncmp(details.str, ".LONG", 5) == 0)
    return T_LONG;
  else if (details.str_size == 5 && strncmp(details.str, ".WORD", 5) == 0)
    return T_WORD;
  else
    return T_NIL;
}

enum UType
{
  U_NIL = -1,
  U_BYTE,
  U_HWORD,
  U_WORD,
} convert_type_to_utype(enum Type type)
{
  if (type == T_CHAR || type == T_INT || type == T_LONG)
    return U_NIL;
  switch (type)
  {
  case T_NIL:
  case T_LONG:
  case T_INT:
  case T_CHAR:
    return U_NIL;
  case T_BYTE:
    return U_BYTE;
  case T_HWORD:
    return U_HWORD;
  case T_WORD:
    return U_WORD;
  }
  return 0;
}

perr_t parse_utype_inst(token_stream_t *stream, inst_t *ret)
{
  if (stream->used + 1 > stream->available)
    return PERR_EXPECTED_OPERAND;
  enum UType type = convert_type_to_utype(
      parse_details_to_type(TOKEN_STREAM_AT(stream->data, stream->used)));
  if (type == U_NIL)
    return PERR_EXPECTED_UTYPE;
  ret->opcode += type;
  return PERR_OK;
}

perr_t parse_type_inst(token_stream_t *stream, inst_t *ret)
{
  if (stream->used + 1 > stream->available)
    return PERR_EXPECTED_OPERAND;
  enum Type type =
      parse_details_to_type(TOKEN_STREAM_AT(stream->data, stream->used));
  if (type == T_NIL)
    return PERR_EXPECTED_TYPE;
  ret->opcode += type;
  return PERR_OK;
}

perr_t parse_utype_inst_with_operand(token_stream_t *stream, inst_t *ret)
{
  perr_t inst_err = parse_utype_inst(stream, ret);
  if (inst_err)
    return inst_err;
  ++stream->used;
  perr_t word_err = parse_word(TOKEN_STREAM_AT(stream->data, stream->used),
                               &ret->operand.as_word);
  if (word_err)
    return word_err;
  return PERR_OK;
}

perr_t parse_type_inst_with_operand(token_stream_t *stream, inst_t *ret)
{
  perr_t inst_err = parse_type_inst(stream, ret);
  if (inst_err)
    return inst_err;
  ++stream->used;
  perr_t word_err = parse_word(TOKEN_STREAM_AT(stream->data, stream->used),
                               &ret->operand.as_word);
  if (word_err)
    return word_err;
  return PERR_OK;
}

perr_t parse_next_inst(token_stream_t *stream, inst_t *ret)
{
  static_assert(NUMBER_OF_OPCODES == 70, "parse_next_inst: Out of date!");
  const token_t token = TOKEN_STREAM_AT(stream->data, stream->used);
  switch (token.type)
  {
  case TOKEN_LITERAL_NUMBER:
  case TOKEN_LITERAL_CHAR:
    return PERR_EXPECTED_SYMBOL;
  case TOKEN_NOOP:
    *ret = INST_NOOP;
    break;
  case TOKEN_HALT:
    *ret = INST_HALT;
    break;
  case TOKEN_PUSH:
    ret->opcode = OP_PUSH_BYTE;
    return parse_utype_inst_with_operand(stream, ret);
  case TOKEN_POP:
    ret->opcode = OP_POP_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_PUSH_REG:
    ret->opcode = OP_PUSH_REGISTER_BYTE;
    return parse_utype_inst_with_operand(stream, ret);
  case TOKEN_MOV:
    ret->opcode = OP_MOV_BYTE;
    return parse_utype_inst_with_operand(stream, ret);
  case TOKEN_DUP:
    ret->opcode = OP_DUP_BYTE;
    return parse_utype_inst_with_operand(stream, ret);
  case TOKEN_NOT:
    ret->opcode = OP_NOT_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_OR:
    ret->opcode = OP_OR_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_AND:
    ret->opcode = OP_AND_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_XOR:
    ret->opcode = OP_XOR_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_EQ:
    ret->opcode = OP_EQ_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_LT:
    ret->opcode = OP_LT_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_LTE:
    ret->opcode = OP_LTE_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_GT:
    ret->opcode = OP_GT_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_GTE:
    ret->opcode = OP_GTE_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_PLUS:
    ret->opcode = OP_PLUS_BYTE;
    return parse_utype_inst(stream, ret);
  case TOKEN_PRINT:
    ret->opcode = OP_PRINT_BYTE;
    return parse_type_inst(stream, ret);
  case TOKEN_JUMP: {
    if (token.str_size == 4 && strncmp(token.str, ".ABS", 4) == 0)
    {
      ret->opcode = OP_JUMP_ABS;
      ++stream->used;
      if (stream->used >= stream->available)
        return PERR_EXPECTED_OPERAND;
      return parse_word(TOKEN_STREAM_AT(stream->data, stream->used),
                        &ret->operand.as_word);
    }
    else if (token.str_size == 9 && strncmp(token.str, ".REGISTER", 9) == 0)
    {
      ret->opcode = OP_JUMP_REGISTER;
      ++stream->used;
      if (stream->used >= stream->available)
        return PERR_EXPECTED_OPERAND;
      return parse_word(TOKEN_STREAM_AT(stream->data, stream->used),
                        &ret->operand.as_word);
    }
    else if (token.str_size == 6 && strncmp(token.str, ".STACK", 6) == 0)
      ret->opcode = OP_JUMP_STACK;
    else
      return PERR_UNKNOWN_OPERATOR;
    break;
  }
  case TOKEN_JUMP_IF: {
    ret->opcode = OP_JUMP_IF_BYTE;
    return parse_utype_inst_with_operand(stream, ret);
  }
  case TOKEN_SYMBOL:
  default:
    return PERR_UNKNOWN_OPERATOR;
  }
  return PERR_OK;
}

perr_t parse_stream(token_stream_t *stream, inst_t **ret, size_t *size)
{
  darr_t instructions = {0};
  darr_init(&instructions, sizeof(inst_t));
  while (stream->used < stream->available)
  {
    inst_t inst = INST_NOOP;
    perr_t err  = parse_next_inst(stream, &inst);
    if (err)
    {
      free(instructions.data);
      return err;
    }
    darr_append_bytes(&instructions, (byte *)&inst, sizeof(inst_t));
    ++stream->used;
  }
  *size = instructions.used / sizeof(inst_t);
  *ret  = (inst_t *)instructions.data;
  return PERR_OK;
}
