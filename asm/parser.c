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
  case PERR_UNKNOWN_LABEL:
    return "UNKNOWN_LABEL";
  case PERR_INVALID_RELATIVE_ADDRESS:
    return "INVALID_RELATIVE_ADDRESS";
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

perr_t parse_sword(token_t token, i64 *ret)
{
  if (token.type == TOKEN_LITERAL_NUMBER)
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
    *ret = i;
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

perr_t parse_word_label_or_relative(token_stream_t *stream, presult_t *res)
{
  token_t token = TOKEN_STREAM_AT(stream->data, stream->used);
  if (token.type == TOKEN_SYMBOL)
  {
    res->type  = PRES_LABEL_ADDRESS;
    res->label = calloc(token.str_size + 1, 1);
    memcpy(res->label, token.str, token.str_size);
    res->label[token.str_size] = '\0';
    return PERR_OK;
  }
  else if (token.type == TOKEN_LITERAL_CHAR ||
           token.type == TOKEN_LITERAL_NUMBER)
  {
    res->type = PRES_COMPLETE_RESULT;
    return parse_word(token, &res->instruction.operand.as_word);
  }
  else if (token.type == TOKEN_STAR)
  {
    if (stream->used + 1 >= stream->available)
      return PERR_EXPECTED_OPERAND;
    res->type = PRES_RELATIVE_ADDRESS;
    ++stream->used;
    return parse_sword(TOKEN_STREAM_AT(stream->data, stream->used),
                       &res->relative_address);
  }
  return PERR_EXPECTED_OPERAND;
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

perr_t parse_jump_inst_operand(token_stream_t *stream, presult_t *res)
{
  perr_t inst_err = parse_utype_inst(stream, &res->instruction);
  if (inst_err)
    return inst_err;
  ++stream->used;
  perr_t op_err = parse_word_label_or_relative(stream, res);
  if (op_err)
    return op_err;
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

perr_t parse_next(token_stream_t *stream, presult_t *ret)
{
  const token_t token = TOKEN_STREAM_AT(stream->data, stream->used);
  perr_t perr         = PERR_OK;
  switch (token.type)
  {
  case TOKEN_LITERAL_NUMBER:
  case TOKEN_LITERAL_CHAR:
    return PERR_EXPECTED_SYMBOL;
  case TOKEN_NOOP:
    *ret = (presult_t){.instruction = INST_NOOP, .type = PRES_COMPLETE_RESULT};
    break;
  case TOKEN_HALT:
    *ret = (presult_t){.instruction = INST_HALT, .type = PRES_COMPLETE_RESULT};
    break;
  case TOKEN_PUSH:
    *ret = (presult_t){.instruction = INST_PUSH(BYTE, 0),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_POP:
    *ret = (presult_t){.instruction = INST_POP(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_PUSH_REG:
    *ret = (presult_t){.instruction = INST_PUSH_REG(BYTE, 0),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_MOV:
    *ret = (presult_t){.instruction = INST_MOV(BYTE, 0),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_DUP:
    *ret = (presult_t){.instruction = INST_DUP(BYTE, 0),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_MALLOC:
    *ret = (presult_t){.instruction = INST_MALLOC(BYTE, 0),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_MSET:
    *ret = (presult_t){.instruction = INST_MSET(BYTE, 0),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_MGET:
    *ret = (presult_t){.instruction = INST_MGET(BYTE, 0),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_MALLOC_STACK:
    *ret = (presult_t){.instruction = INST_MALLOC_STACK(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_MSET_STACK:
    *ret = (presult_t){.instruction = INST_MSET_STACK(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_MGET_STACK:
    *ret = (presult_t){.instruction = INST_MGET_STACK(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_MDELETE:
    *ret =
        (presult_t){.instruction = INST_MDELETE, .type = PRES_COMPLETE_RESULT};
    break;
  case TOKEN_MSIZE:
    *ret = (presult_t){.instruction = INST_MSIZE, .type = PRES_COMPLETE_RESULT};
    break;
  case TOKEN_NOT:
    *ret = (presult_t){.instruction = INST_NOT(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_OR:
    *ret =
        (presult_t){.instruction = INST_OR(BYTE), .type = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_AND:
    *ret = (presult_t){.instruction = INST_AND(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_XOR:
    *ret = (presult_t){.instruction = INST_XOR(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_EQ:
    *ret =
        (presult_t){.instruction = INST_EQ(BYTE), .type = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_LT:
    *ret =
        (presult_t){.instruction = INST_LT(BYTE), .type = PRES_COMPLETE_RESULT};
    perr = parse_type_inst(stream, &ret->instruction);
    break;
  case TOKEN_LTE:
    *ret = (presult_t){.instruction = INST_LTE(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_type_inst(stream, &ret->instruction);
    break;
  case TOKEN_GT:
    *ret =
        (presult_t){.instruction = INST_GT(BYTE), .type = PRES_COMPLETE_RESULT};
    perr = parse_type_inst(stream, &ret->instruction);
    break;
  case TOKEN_GTE:
    *ret = (presult_t){.instruction = INST_GTE(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_type_inst(stream, &ret->instruction);
    break;
  case TOKEN_PLUS:
    *ret = (presult_t){.instruction = INST_PLUS(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_SUB:
    *ret = (presult_t){.instruction = INST_SUB(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_MULT:
    *ret = (presult_t){.instruction = INST_MULT(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_PRINT:
    *ret = (presult_t){.instruction = INST_PRINT(BYTE),
                       .type        = PRES_COMPLETE_RESULT};
    perr = parse_type_inst(stream, &ret->instruction);
    break;
  case TOKEN_JUMP: {
    if (token.str_size == 4 && strncmp(token.str, ".ABS", 4) == 0)
    {
      *ret = (presult_t){.instruction = INST_JUMP_ABS(0)};
      ++stream->used;
      if (stream->used >= stream->available)
        return PERR_EXPECTED_OPERAND;
      return parse_word_label_or_relative(stream, ret);
    }
    else if (token.str_size == 9 && strncmp(token.str, ".REGISTER", 9) == 0)
    {
      *ret = (presult_t){.instruction = INST_JUMP_REGISTER(0),
                         .type        = PRES_COMPLETE_RESULT};
      ++stream->used;
      if (stream->used >= stream->available)
        return PERR_EXPECTED_OPERAND;
      return parse_word(TOKEN_STREAM_AT(stream->data, stream->used),
                        &ret->instruction.operand.as_word);
    }
    else if (token.str_size == 6 && strncmp(token.str, ".STACK", 6) == 0)
      *ret = (presult_t){.instruction = INST_JUMP_STACK,
                         .type        = PRES_COMPLETE_RESULT};
    else
      return PERR_UNKNOWN_OPERATOR;
    break;
  }
  case TOKEN_JUMP_IF: {
    *ret = (presult_t){.instruction = INST_JUMP_IF(BYTE, 0)};
    return parse_jump_inst_operand(stream, ret);
  }
  case TOKEN_SYMBOL: {
    size_t label_size = strcspn(token.str, ":");
    if (label_size == strlen(token.str))
      return PERR_UNKNOWN_OPERATOR;
    *ret       = (presult_t){.type = PRES_LABEL};
    ret->label = calloc(label_size + 1, 1);
    memcpy(ret->label, token.str, label_size);
    ret->label[label_size] = '\0';
    break;
  }
  case TOKEN_STAR:
  default:
    return PERR_UNKNOWN_OPERATOR;
  }
  return perr;
}

struct LabelPair
{
  char *label;
  size_t label_size;
  word addr;
};

perr_t process_presults(presult_t *results, size_t res_count,
                        inst_t **instructions, size_t *inst_count)
{
  darr_t label_pairs = {0};
  darr_init(&label_pairs, sizeof(struct LabelPair));
  *inst_count = 0;
  for (size_t i = 0; i < res_count; ++i)
  {
    presult_t res = results[i];
    switch (res.type)
    {
    case PRES_LABEL: {
      struct LabelPair pair = {0};
      pair.label            = res.label;
      pair.addr             = (*inst_count);
      pair.label_size       = strlen(res.label);
      darr_append_bytes(&label_pairs, (byte *)&pair, sizeof(pair));
      break;
    }
    case PRES_RELATIVE_ADDRESS: {
      s_word offset = res.relative_address;
      if (offset < 0 && ((word)(-offset)) > *inst_count)
      {
        free(label_pairs.data);
        return PERR_INVALID_RELATIVE_ADDRESS;
      }
      results[i].instruction.operand.as_word = ((s_word)*inst_count) + offset;
      (*inst_count)++;
      break;
    }
    case PRES_LABEL_ADDRESS:
    case PRES_COMPLETE_RESULT:
    default: {
      (*inst_count)++;
      break;
    }
    }
  }

  darr_t instr_darr = {0};
  darr_init(&instr_darr, sizeof(**instructions));
  for (size_t i = 0; i < res_count; ++i)
  {
    presult_t res = results[i];
    switch (res.type)
    {
    case PRES_LABEL_ADDRESS: {
      inst_t inst = {0};
      for (size_t j = 0; j < (label_pairs.used / sizeof(struct LabelPair)); ++j)
      {
        struct LabelPair pair = ((struct LabelPair *)label_pairs.data)[j];
        if (pair.label_size == strlen(res.label) &&
            strncmp(pair.label, res.label, pair.label_size) == 0)
        {
          inst         = res.instruction;
          inst.operand = DWORD(pair.addr);
        }
      }

      if (inst.opcode == OP_NOOP)
      {
        free(instr_darr.data);
        free(label_pairs.data);
        return PERR_UNKNOWN_LABEL;
      }
      darr_append_bytes(&instr_darr, (byte *)&inst, sizeof(inst));
      break;
    }
    case PRES_RELATIVE_ADDRESS:
    case PRES_COMPLETE_RESULT:
      darr_append_bytes(&instr_darr, (byte *)&res.instruction,
                        sizeof(res.instruction));
    case PRES_LABEL:
      break;
    }
  }

  free(label_pairs.data);
  *instructions = (inst_t *)instr_darr.data;
  return PERR_OK;
}

perr_t parse_stream(token_stream_t *stream, inst_t **ret, size_t *size)
{
  darr_t presults = {0};
  darr_init(&presults, sizeof(presult_t));
  while (stream->used < stream->available)
  {
    presult_t pres = {0};
    perr_t err     = parse_next(stream, &pres);
    if (err)
    {
      for (size_t i = 0; i < (presults.used / sizeof(presult_t)); ++i)
      {
        presult_t res = ((presult_t *)presults.data)[i];
        if (res.type == PRES_LABEL_ADDRESS || res.type == PRES_LABEL)
          free(res.label);
      }
      free(presults.data);
      return err;
    }
    darr_append_bytes(&presults, (byte *)&pres, sizeof(presult_t));
    ++stream->used;
  }

  perr_t perr = process_presults((presult_t *)presults.data,
                                 presults.used / sizeof(presult_t), ret, size);
  for (size_t i = 0; i < (presults.used / sizeof(presult_t)); ++i)
  {
    presult_t res = ((presult_t *)presults.data)[i];
    if (res.type == PRES_LABEL_ADDRESS || res.type == PRES_LABEL)
      free(res.label);
  }
  free(presults.data);
  return perr;
}
