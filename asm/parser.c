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

opcode_t get_typed_opcode(opcode_t base_code, data_type_t type)
{
  switch (type)
  {
  case DATA_TYPE_NIL:
    // TODO: Parse error (Not something we should pass here)
    return 0;
  case DATA_TYPE_BYTE:
    return base_code;
  case DATA_TYPE_HWORD:
    return base_code + 1;
  case DATA_TYPE_WORD:
    return base_code + 2;
  default:
    // TODO: Parse error (EXPECTED_TYPE_TAG)
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

word parse_word(token_t token)
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
      // TODO: Parse error (NOT_A_NUMBER)
      return 0;
    else if (errno == ERANGE)
    {
      // TODO: Parse error (INTEGER_OVERFLOW)
      errno = 0;
      return 0;
    }
    memcpy(&w, &i, sizeof(w));
  }
  else
  {
    char *end = NULL;
    w         = strtoull(token.str, &end, 0);
    if (!(end && end[0] == '\0'))
      // TODO: Parse error (NOT_A_NUMBER)
      return 0;
    else if (errno == ERANGE)
    {
      // TODO: Parse error (WORD_OVERFLOW)
      errno = 0;
      return 0;
    }
  }
  return w;
}

inst_t parse_next_inst(token_stream_t *stream)
{
  token_t token = TOKEN_STREAM_AT(stream->data, stream->used);
  if (token.type != TOKEN_SYMBOL)
    // TODO Parser Error (EXPECTED_SYMBOL)
    return INST_NOOP;
  inst_t inst  = {0};
  char *opcode = token.str;
  if (token.str_size >= 4 && strncmp(opcode, "HALT", 4) == 0)
  {
    inst = INST_HALT;
    ++stream->used;
  }
  else if (token.str_size >= 5 && strncmp(opcode, "PUSH.", 5) == 0)
  {
    data_type_t type = parse_data_type(opcode + 5, token.str_size - 5);
    inst.opcode      = get_typed_opcode(OP_PUSH_BYTE, type);
    if (!OPCODE_IS_TYPE(inst.opcode, OP_PUSH))
      // TODO: Parse error (EXPECTED_TYPE_TAG)
      return INST_NOOP;
    else if (stream->used == stream->available - 1)
      // TODO: Parse error (EXPECTED_OPERAND)
      return INST_NOOP;
    inst.operand =
        DWORD(parse_word(TOKEN_STREAM_AT(stream->data, stream->used + 1)));
    stream->used += 2;
  }
  else if (token.str_size >= 4 && strncmp(opcode, "POP.", 4) == 0)
  {
    data_type_t type = parse_data_type(opcode + 4, token.str_size - 4);
    inst.opcode      = get_typed_opcode(OP_POP_BYTE, type);
    if (!OPCODE_IS_TYPE(inst.opcode, OP_POP))
      // TODO: Parse error (EXPECTED_TYPE_TAG)
      return INST_NOOP;
    ++stream->used;
  }
  else if (token.str_size >= 9 && strncmp(opcode, "PUSH-REG.", 9) == 0)
  {
    data_type_t type = parse_data_type(opcode + 9, token.str_size - 9);
    inst.opcode      = get_typed_opcode(OP_PUSH_REGISTER_BYTE, type);
    if (!OPCODE_IS_TYPE(inst.opcode, OP_PUSH_REGISTER))
      // TODO: Parse error (EXPECTED_TYPE_TAG)
      return INST_NOOP;
    else if (stream->used == stream->available - 1)
      // TODO: Parse error (EXPECTED_OPERAND)
      return INST_NOOP;
    inst.operand =
        DWORD(parse_word(TOKEN_STREAM_AT(stream->data, stream->used + 1)));
    stream->used += 2;
  }
  else if (token.str_size >= 4 && strncmp(opcode, "MOV.", 4) == 0)
  {
    data_type_t type = parse_data_type(opcode + 4, token.str_size - 4);
    inst.opcode      = get_typed_opcode(OP_MOV_BYTE, type);
    if (!OPCODE_IS_TYPE(inst.opcode, OP_MOV))
      // TODO: Parse error (EXPECTED_TYPE_TAG)
      return INST_NOOP;
    else if (stream->used == stream->available - 1)
      // TODO: Parse error (EXPECTED_OPERAND)
      return INST_NOOP;
    inst.operand =
        DWORD(parse_word(TOKEN_STREAM_AT(stream->data, stream->used + 1)));
    stream->used += 2;
  }
  else if (token.str_size >= 4 && strncmp(opcode, "DUP.", 4) == 0)
  {
    data_type_t type = parse_data_type(opcode + 4, token.str_size - 4);
    inst.opcode      = get_typed_opcode(OP_DUP_BYTE, type);
    if (!OPCODE_IS_TYPE(inst.opcode, OP_DUP))
      // TODO: Parse error (EXPECTED_TYPE_TAG)
      return INST_NOOP;
    else if (stream->used == stream->available - 1)
      // TODO: Parse error (EXPECTED_OPERAND)
      return INST_NOOP;
    inst.operand =
        DWORD(parse_word(TOKEN_STREAM_AT(stream->data, stream->used + 1)));
    stream->used += 2;
  }
  else
  {
    // TODO: Parse error (UNKNOWN_OPCODE)
    return INST_NOOP;
  }
  return inst;
}

inst_t *parse_stream(token_stream_t *stream, size_t *size)
{
  darr_t instructions = {0};
  darr_init(&instructions, sizeof(inst_t));
  while (stream->used < stream->available)
  {
    inst_t inst = parse_next_inst(stream);
    darr_append_bytes(&instructions, (byte *)&inst, sizeof(inst_t));
  }
  *size = instructions.used / sizeof(inst_t);
  return (inst_t *)instructions.data;
}
