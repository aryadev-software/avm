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

opcode_t get_typed_opcode(opcode_t base_code, data_type_t type)
{
  switch (type)
  {
  case DATA_TYPE_BYTE:
    return base_code;
  case DATA_TYPE_HWORD:
    return base_code + 1;
  case DATA_TYPE_WORD:
    return base_code + 2;
  case DATA_TYPE_NIL:
  default:
    return 0;
  }
}

data_type_t parse_data_type(const char *cstr, size_t length)
{
  if (length >= 4 && strncmp(cstr, "BYTE", 4) == 0)
    return DATA_TYPE_BYTE;
  else if (length >= 5 && strncmp(cstr, "HWORD", 5) == 0)
    return DATA_TYPE_HWORD;
  else if (length >= 4 && strncmp(cstr, "WORD", 4) == 0)
    return DATA_TYPE_WORD;
  else
    return DATA_TYPE_NIL;
}

perr_t parse_word(token_t token, word *ret)
{
  assert(token.type == TOKEN_LITERAL_NUMBER);
  bool is_negative = token.str_size > 1 && token.str[0] == '-';
  word w           = 0;
  if (is_negative)
  {
    char *end = NULL;
    // TODO: Make a standardised type of the same size as word in
    // base.h
    int64_t i = strtoll(token.str, &end, 0);
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

perr_t parse_inst_with_type(token_stream_t *stream, inst_t *ret,
                            size_t oplength)
{
  // Assume the base type OP_*_BYTE is in ret->opcode
  token_t token    = TOKEN_STREAM_AT(stream->data, stream->used);
  char *opcode     = token.str;
  data_type_t type = parse_data_type(opcode + oplength,
                                     WORD_SAFE_SUB(token.str_size, oplength));
  if (type == DATA_TYPE_NIL)
    return PERR_EXPECTED_TYPE;
  ++stream->used;
  ret->opcode = get_typed_opcode(ret->opcode, type);
  return PERR_OK;
}

perr_t parse_inst_with_operand(token_stream_t *stream, inst_t *ret)
{
  // Parse operand
  perr_t word_parse_error = parse_word(
      TOKEN_STREAM_AT(stream->data, stream->used), &ret->operand.as_word);
  if (word_parse_error)
    return word_parse_error;
  ++stream->used;
  return PERR_OK;
}

perr_t parse_inst_with_typed_operand(token_stream_t *stream, inst_t *ret,
                                     size_t oplength)
{
  perr_t type_parse_error = parse_inst_with_type(stream, ret, oplength);
  if (type_parse_error)
    return type_parse_error;

  // Parse operand
  perr_t word_parse_error = parse_word(
      TOKEN_STREAM_AT(stream->data, stream->used), &ret->operand.as_word);
  if (word_parse_error)
    return word_parse_error;
  ++stream->used;
  return PERR_OK;
}

perr_t parse_next_inst(token_stream_t *stream, inst_t *ret)
{
  const token_t token = TOKEN_STREAM_AT(stream->data, stream->used);
  if (token.type != TOKEN_SYMBOL)
    return PERR_EXPECTED_SYMBOL;
  inst_t inst  = {0};
  char *opcode = token.str;
  if (token.str_size == 4 && strncmp(opcode, "NOOP", 4) == 0)
  {
    inst = INST_NOOP;
    ++stream->used;
  }
  else if (token.str_size == 4 && strncmp(opcode, "HALT", 4) == 0)
  {
    inst = INST_HALT;
    ++stream->used;
  }
  else if (token.str_size >= 4 && strncmp(opcode, "PUSH", 4) == 0)
  {
    size_t oplen = 5;
    if (token.str_size >= 8 && strncmp(opcode, "PUSH-REG", 8) == 0)
    {
      oplen       = 9;
      ret->opcode = OP_PUSH_REGISTER_BYTE;
    }
    else
      ret->opcode = OP_PUSH_BYTE;
    return parse_inst_with_typed_operand(stream, ret, oplen);
  }
  else if (token.str_size >= 3 && strncmp(opcode, "POP", 3) == 0)
  {
    ret->opcode = OP_POP_BYTE;
    return parse_inst_with_type(stream, ret, 4);
  }
  else if (token.str_size >= 3 && strncmp(opcode, "MOV", 3) == 0)
  {
    ret->opcode = OP_MOV_BYTE;
    return parse_inst_with_typed_operand(stream, ret, 4);
  }
  else if (token.str_size >= 3 && strncmp(opcode, "DUP", 3) == 0)
  {
    ret->opcode = OP_DUP_BYTE;
    return parse_inst_with_typed_operand(stream, ret, 4);
  }
  else if (token.str_size >= 3 && strncmp(opcode, "NOT", 3) == 0)
  {
    ret->opcode = OP_NOT_BYTE;
    return parse_inst_with_type(stream, ret, 4);
  }
  else if (token.str_size >= 2 && strncmp(opcode, "OR", 2) == 0)
  {
    ret->opcode = OP_OR_BYTE;
    return parse_inst_with_type(stream, ret, 3);
  }
  else if (token.str_size >= 3 && strncmp(opcode, "AND", 3) == 0)
  {
    ret->opcode = OP_AND_BYTE;
    return parse_inst_with_type(stream, ret, 4);
  }
  else if (token.str_size >= 3 && strncmp(opcode, "XOR", 3) == 0)
  {
    ret->opcode = OP_XOR_BYTE;
    return parse_inst_with_type(stream, ret, 4);
  }
  else if (token.str_size >= 2 && strncmp(opcode, "EQ", 2) == 0)
  {
    ret->opcode = OP_EQ_BYTE;
    return parse_inst_with_type(stream, ret, 3);
  }
  else if (token.str_size >= 4 && strncmp(opcode, "PLUS", 4) == 0)
  {
    ret->opcode = OP_PLUS_BYTE;
    return parse_inst_with_type(stream, ret, 5);
  }
  else if (token.str_size >= 6 && strncmp(opcode, "PRINT.", 6) == 0)
  {
    const char *type       = opcode + 6;
    const size_t type_size = WORD_SAFE_SUB(token.str_size, 6);
    if (type_size == 4 && strncmp(type, "CHAR", 4) == 0)
      inst.opcode = OP_PRINT_CHAR;
    else if (type_size == 4 && strncmp(type, "BYTE", 4) == 0)
      inst.opcode = OP_PRINT_BYTE;
    else if (type_size == 3 && strncmp(type, "INT", 3) == 0)
      inst.opcode = OP_PRINT_INT;
    else if (type_size == 5 && strncmp(type, "HWORD", 5) == 0)
      inst.opcode = OP_PRINT_HWORD;
    else if (type_size == 4 && strncmp(type, "LONG", 4) == 0)
      inst.opcode = OP_PRINT_LONG;
    else if (type_size == 4 && strncmp(type, "WORD", 4) == 0)
      inst.opcode = OP_PRINT_WORD;
    else
      return PERR_UNKNOWN_OPERATOR;
    ++stream->used;
  }
  else if (token.str_size >= 5 && strncmp(opcode, "JUMP.", 5) == 0)
  {
    const char *type       = opcode + 5;
    const size_t type_size = WORD_SAFE_SUB(token.str_size, 5);
    if (type_size == 3 && strncmp(type, "ABS", 3) == 0)
    {
      ret->opcode = OP_JUMP_ABS;
      ++stream->used;
      return parse_inst_with_operand(stream, ret);
    }
    else if (type_size == 5 && strncmp(type, "STACK", 5) == 0)
      inst.opcode = OP_JUMP_STACK;
    else if (type_size == 8 && strncmp(type, "REGISTER", 8) == 0)
    {
      ret->opcode = OP_JUMP_REGISTER;
      ++stream->used;
      return parse_inst_with_operand(stream, ret);
    }
    else if (type_size >= 2 && strncmp(type, "IF", 2) == 0)
    {
      // Parse a typed operand JUMP.IF.<TYPE>
      token_t prev    = TOKEN_STREAM_AT(stream->data, stream->used);
      size_t prev_ptr = stream->used;

      TOKEN_STREAM_AT(stream->data, stream->used).str      = (char *)type;
      TOKEN_STREAM_AT(stream->data, stream->used).str_size = type_size;
      ret->opcode                                          = OP_JUMP_IF_BYTE;
      perr_t perr = parse_inst_with_typed_operand(stream, ret, 3);

      TOKEN_STREAM_AT(stream->data, prev_ptr) = prev;
      return perr;
    }
    else
      return PERR_UNKNOWN_OPERATOR;
    ++stream->used;
  }
  else
    return PERR_UNKNOWN_OPERATOR;
  *ret = inst;
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
  }
  *size = instructions.used / sizeof(inst_t);
  *ret  = (inst_t *)instructions.data;
  return PERR_OK;
}
