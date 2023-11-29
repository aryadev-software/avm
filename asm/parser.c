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
  case PERR_EXPECTED_LABEL:
    return "EXPECTED_LABEL";
  case PERR_EXPECTED_OPERAND:
    return "EXPECTED_OPERAND";
  case PERR_PREPROCESSOR_EXPECTED_STRING:
    return "PREPROCESSOR_EXPECTED_STRING";
  case PERR_PREPROCESSOR_FILE_NONEXISTENT:
    return "PREPROCESSOR_FILE_NONEXISTENT";
  case PERR_PREPROCESSOR_FILE_PARSE_ERROR:
    return "PREPROCESSOR_FILE_PARSE_ERROR";
  case PERR_PREPROCESSOR_EXPECTED_END:
    return "PREPROCESSOR_EXPECTED_END";
  case PERR_PREPROCESSOR_EXPECTED_NAME:
    return "PREPROCESSOR_EXPECTED_NAME";
  case PERR_PREPROCESSOR_UNKNOWN_NAME:
    return "PREPROCESSOR_UNKNOWN_NAME";
  case PERR_INVALID_RELATIVE_ADDRESS:
    return "INVALID_RELATIVE_ADDRESS";
  case PERR_UNKNOWN_LABEL:
    return "UNKNOWN_LABEL";
  case PERR_UNKNOWN_OPERATOR:
    return "UNKNOWN_OPERATOR";
  default:
    return "";
  }
}

presult_t presult_label(size_t stream_index, const char *name, size_t size,
                        s_word addr)
{
  presult_t res = {.stream_index = stream_index,
                   .address      = addr,
                   .label        = {.name = malloc(size + 1), .size = size}};
  memcpy(res.label.name, name, size);
  res.label.name[size] = '\0';
  return res;
}

presult_t presult_label_ref(size_t stream_index, inst_t base, const char *label,
                            size_t size)
{
  presult_t pres   = presult_label(stream_index, label, size, 0);
  pres.instruction = base;
  pres.type        = PRES_LABEL_ADDRESS;
  return pres;
}

presult_t presult_instruction(size_t stream_index, inst_t inst)
{
  return (presult_t){.stream_index = stream_index,
                     .instruction  = inst,
                     .type         = PRES_COMPLETE_RESULT};
}

presult_t presult_relative(size_t stream_index, inst_t inst, s_word addr)
{
  return (presult_t){.stream_index = stream_index,
                     .instruction  = inst,
                     .address      = addr,
                     .type         = PRES_RELATIVE_ADDRESS};
}

presult_t presult_global(size_t stream_index, const char *name, size_t size,
                         s_word addr)
{
  presult_t res = presult_label(stream_index, name, size, addr);
  res.type      = PRES_GLOBAL_LABEL;
  return res;
}

void presult_free(presult_t res)
{
  switch (res.type)
  {
  case PRES_LABEL_ADDRESS:
  case PRES_GLOBAL_LABEL:
  case PRES_LABEL:
    free(res.label.name);
    break;
  case PRES_RELATIVE_ADDRESS:
  case PRES_COMPLETE_RESULT:
    break;
  }
}

void presults_free(presult_t *ptr, size_t number)
{
  for (size_t i = 0; i < number; ++i)
    presult_free(ptr[i]);
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
    *res = presult_label_ref(stream->used, res->instruction, token.str,
                             token.str_size);
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
                       &res->address);
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
  if (details.str_size == 4 && strncmp(details.str, "BYTE", 4) == 0)
    return T_BYTE;
  else if (details.str_size == 4 && strncmp(details.str, "CHAR", 4) == 0)
    return T_CHAR;
  else if (details.str_size == 5 && strncmp(details.str, "HWORD", 5) == 0)
    return T_HWORD;
  else if (details.str_size == 3 && strncmp(details.str, "INT", 3) == 0)
    return T_INT;
  else if (details.str_size == 4 && strncmp(details.str, "LONG", 4) == 0)
    return T_LONG;
  else if (details.str_size == 4 && strncmp(details.str, "WORD", 4) == 0)
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

label_t search_labels(label_t *labels, size_t n, char *name, size_t name_size)
{
  for (size_t i = 0; i < n; ++i)
  {
    label_t label = labels[i];
    if (label.name_size == name_size &&
        strncmp(label.name, name, name_size) == 0)
      return label;
  }

  return (label_t){0};
}

block_t search_blocks(block_t *blocks, size_t n, char *name, size_t name_size)
{
  for (size_t i = 0; i < n; ++i)
  {
    block_t block = blocks[i];
    if (block.name_size == name_size &&
        strncmp(block.name, name, name_size) == 0)
      return block;
  }

  return (block_t){0};
}

perr_t preprocess_use_blocks(token_stream_t *stream, token_stream_t *new)
{
  token_stream_t new_stream = {0};
  darr_init(&new_stream, sizeof(token_t));
  // %USE <STRING FILENAME> -> #TOKENS_IN(FILENAME)
  for (size_t i = 0; i < stream->available; ++i)
  {
    token_t t = DARR_AT(token_t, stream->data, i);
    if (t.type == TOKEN_PP_USE)
    {
      if (i + 1 >= stream->available ||
          DARR_AT(token_t, stream->data, i + 1).type != TOKEN_LITERAL_STRING)
      {
        stream->used = i + 1 >= stream->available ? i : i + 1;
        for (size_t i = 0; i < (new_stream.available / sizeof(token_t)); ++i)
          free(TOKEN_STREAM_AT(new_stream.data, i).str);
        free(new_stream.data);
        return PERR_PREPROCESSOR_EXPECTED_STRING;
      }
      // Load and tokenise another file
      ++i;
      t        = DARR_AT(token_t, stream->data, i);
      FILE *fp = fopen(t.str, "rb");
      if (!fp)
      {
        for (size_t i = 0; i < new_stream.available; ++i)
          free(TOKEN_STREAM_AT(new_stream.data, i).str);
        free(new_stream.data);
        stream->used = i;
        return PERR_PREPROCESSOR_FILE_NONEXISTENT;
      }
      buffer_t buffer = darr_read_file(fp);
      fclose(fp);

      token_stream_t fstream = {0};
      lerr_t lerr            = tokenise_buffer(&buffer, &fstream);
      free(buffer.data);
      if (lerr)
      {
        if (fstream.data)
        {
          for (size_t i = 0; i < fstream.available; ++i)
            free(TOKEN_STREAM_AT(fstream.data, i).str);
          free(fstream.data);
        }
        for (size_t i = 0; i < new_stream.available; ++i)
          free(TOKEN_STREAM_AT(new_stream.data, i).str);
        free(new_stream.data);
        stream->used = i;
        return PERR_PREPROCESSOR_FILE_PARSE_ERROR;
      }
      darr_append_bytes(&new_stream, fstream.data,
                        sizeof(token_t) * fstream.available);
      free(fstream.data);
    }
    else
    {
      token_t copy = token_copy(t);
      darr_append_bytes(&new_stream, (byte *)&copy, sizeof(copy));
    }
  }

  new_stream.available = new_stream.used / sizeof(token_t);
  new_stream.used      = 0;
  *new                 = new_stream;

  return PERR_OK;
}

perr_t preprocess_macro_blocks(token_stream_t *stream, token_stream_t *new)
{
  darr_t block_registry = {0};
  darr_init(&block_registry, sizeof(block_t));

  for (size_t i = 0; i < stream->available; ++i)
  {
    token_t t = DARR_AT(token_t, stream->data, i);
    if (t.type == TOKEN_PP_CONST)
    {
      char *sym    = t.str;
      size_t start = strcspn(sym, "(");
      size_t end   = strcspn(sym, ")");
      if (end == t.str_size || start == t.str_size || start == end + 1)
      {
        free(block_registry.data);
        return PERR_PREPROCESSOR_EXPECTED_NAME;
      }
      block_t block = {.name = sym + start + 1, .name_size = end - start - 1};
      ++i;
      size_t prev = i;
      token_t t   = {0};
      for (t = DARR_AT(token_t, stream->data, i);
           i < stream->available && t.type != TOKEN_PP_END;
           ++i, t = DARR_AT(token_t, stream->data, i))
        continue;
      if (t.type != TOKEN_PP_END)
      {
        stream->used = i;
        free(block_registry.data);
        return PERR_PREPROCESSOR_EXPECTED_END;
      }

      // Set the block's token DARR by hand
      block.code.data      = stream->data + (prev * sizeof(token_t));
      block.code.available = i - prev;
      block.code.used      = block.code.available;
      darr_append_bytes(&block_registry, (byte *)&block, sizeof(block));
    }
  }

  if (block_registry.used == 0)
  {
    // Nothing to preprocess so just copywholesale
    free(block_registry.data);
    *new = (token_stream_t){0};
    darr_init(new, sizeof(token_t));
    for (size_t i = 0; i < stream->available; ++i)
    {
      token_t token = DARR_AT(token_t, stream->data, i);
      token_t copy  = token_copy(token);
      darr_append_bytes(new, (byte *)&copy, sizeof(copy));
    }
    new->available = new->used / sizeof(token_t);
    new->used      = 0;
    return PERR_OK;
  }

  // Stream with blocks now inlined
  token_stream_t new_stream = {0};
  darr_init(&new_stream, sizeof(token_t));
  for (size_t i = 0; i < stream->available; ++i)
  {
    token_t t = DARR_AT(token_t, stream->data, i);
    if (t.type == TOKEN_PP_CONST)
    {
      // Skip till after end
      for (; i < stream->available && t.type != TOKEN_PP_END;
           ++i, t = DARR_AT(token_t, stream->data, i))
        continue;
    }
    else if (t.type == TOKEN_PP_REFERENCE)
    {
      // Find the reference in the block registry
      block_t block = search_blocks((block_t *)block_registry.data,
                                    block_registry.used, t.str, t.str_size);
      if (!block.name)
      {
        free(new_stream.data);
        free(block_registry.data);
        stream->used = i;
        return PERR_PREPROCESSOR_UNKNOWN_NAME;
      }

      // Inline the block found
      for (size_t j = 0; j < block.code.used; j++)
      {
        token_t b_token = DARR_AT(token_t, block.code.data, j);
        token_t copy    = token_copy(b_token);
        darr_append_bytes(&new_stream, (byte *)&copy, sizeof(token_t));
      }
    }
    else
    {
      // Insert into stream as is
      token_t copy = token_copy(t);
      darr_append_bytes(&new_stream, (byte *)&copy, sizeof(copy));
    }
  }

  // Free block registry
  free(block_registry.data);

  new_stream.available = new_stream.used / sizeof(token_t);
  new_stream.used      = 0;
  *new                 = new_stream;

  return PERR_OK;
}

perr_t preprocessor(token_stream_t *stream)
{
  token_stream_t use_blocks = {0};
  perr_t perr               = preprocess_use_blocks(stream, &use_blocks);
  if (perr)
    return perr;

  token_stream_t macro_blocks = {0};
  perr = preprocess_macro_blocks(&use_blocks, &macro_blocks);
  if (perr)
  {
    stream->used = use_blocks.used;
    for (size_t i = 0; i < use_blocks.available; ++i)
      free(TOKEN_STREAM_AT(use_blocks.data, i).str);
    free(use_blocks.data);
    return perr;
  }

  for (size_t i = 0; i < use_blocks.available; ++i)
    free(TOKEN_STREAM_AT(use_blocks.data, i).str);
  free(use_blocks.data);

  for (size_t i = 0; i < stream->available; ++i)
    free(TOKEN_STREAM_AT(stream->data, i).str);
  free(stream->data);

  *stream = macro_blocks;

  return PERR_OK;
}

perr_t parse_next(token_stream_t *stream, presult_t *ret)
{
  token_t token = TOKEN_STREAM_AT(stream->data, stream->used);
  perr_t perr   = PERR_OK;
  switch (token.type)
  {
  case TOKEN_LITERAL_STRING:
  case TOKEN_PP_CONST:
  case TOKEN_PP_USE:
  case TOKEN_PP_REFERENCE:
  case TOKEN_PP_END:
  case TOKEN_LITERAL_NUMBER:
  case TOKEN_LITERAL_CHAR:
    return PERR_EXPECTED_SYMBOL;
  case TOKEN_GLOBAL: {
    if (stream->used + 1 >= stream->available ||
        TOKEN_STREAM_AT(stream->data, stream->used + 1).type != TOKEN_SYMBOL)
      return PERR_EXPECTED_LABEL;
    ++stream->used;
    token_t label = TOKEN_STREAM_AT(stream->data, stream->used);
    *ret          = presult_global(stream->used, label.str, label.str_size, 0);
    return PERR_OK;
  }
  case TOKEN_NOOP:
    *ret      = presult_instruction(stream->used, INST_NOOP);
    ret->type = PRES_COMPLETE_RESULT;
    break;
  case TOKEN_HALT:
    *ret      = presult_instruction(stream->used, INST_HALT);
    ret->type = PRES_COMPLETE_RESULT;
    break;
  case TOKEN_PUSH:
    *ret = presult_instruction(stream->used, INST_PUSH(BYTE, 0));
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_POP:
    *ret = presult_instruction(stream->used, INST_POP(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_PUSH_REG:
    *ret = presult_instruction(stream->used, INST_PUSH_REG(BYTE, 0));
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_MOV:
    *ret = presult_instruction(stream->used, INST_MOV(BYTE, 0));
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_DUP:
    *ret = presult_instruction(stream->used, INST_DUP(BYTE, 0));
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_MALLOC:
    *ret = presult_instruction(stream->used, INST_MALLOC(BYTE, 0));
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_MSET:
    *ret = presult_instruction(stream->used, INST_MSET(BYTE, 0));
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_MGET:
    *ret = presult_instruction(stream->used, INST_MGET(BYTE, 0));
    perr = parse_utype_inst_with_operand(stream, &ret->instruction);
    break;
  case TOKEN_MALLOC_STACK:
    *ret = presult_instruction(stream->used, INST_MALLOC_STACK(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_MSET_STACK:
    *ret = presult_instruction(stream->used, INST_MSET_STACK(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_MGET_STACK:
    *ret = presult_instruction(stream->used, INST_MGET_STACK(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_MDELETE:
    *ret = presult_instruction(stream->used, INST_MDELETE);
    break;
  case TOKEN_MSIZE:
    *ret = presult_instruction(stream->used, INST_MSIZE);
    break;
  case TOKEN_NOT:
    *ret = presult_instruction(stream->used, INST_NOT(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_OR:
    *ret = presult_instruction(stream->used, INST_OR(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_AND:
    *ret = presult_instruction(stream->used, INST_AND(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_XOR:
    *ret = presult_instruction(stream->used, INST_XOR(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_EQ:
    *ret = presult_instruction(stream->used, INST_EQ(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_LT:
    *ret = presult_instruction(stream->used, INST_LT(BYTE));
    perr = parse_type_inst(stream, &ret->instruction);
    break;
  case TOKEN_LTE:
    *ret = presult_instruction(stream->used, INST_LTE(BYTE));
    perr = parse_type_inst(stream, &ret->instruction);
    break;
  case TOKEN_GT:
    *ret = presult_instruction(stream->used, INST_GT(BYTE));
    perr = parse_type_inst(stream, &ret->instruction);
    break;
  case TOKEN_GTE:
    *ret = presult_instruction(stream->used, INST_GTE(BYTE));
    perr = parse_type_inst(stream, &ret->instruction);
    break;
  case TOKEN_PLUS:
    *ret = presult_instruction(stream->used, INST_PLUS(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_SUB:
    *ret = presult_instruction(stream->used, INST_SUB(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_MULT:
    *ret = presult_instruction(stream->used, INST_MULT(BYTE));
    perr = parse_utype_inst(stream, &ret->instruction);
    break;
  case TOKEN_PRINT:
    *ret = presult_instruction(stream->used, INST_PRINT(BYTE));
    perr = parse_type_inst(stream, &ret->instruction);
    break;
  case TOKEN_JUMP_ABS:
    *ret = presult_instruction(stream->used, INST_JUMP_ABS(0));
    ++stream->used;
    if (stream->used >= stream->available)
      return PERR_EXPECTED_OPERAND;
    return parse_word_label_or_relative(stream, ret);
  case TOKEN_JUMP_STACK:
    *ret = presult_instruction(stream->used, INST_JUMP_STACK);
    break;
  case TOKEN_JUMP_IF: {
    *ret = presult_instruction(stream->used, INST_JUMP_IF(BYTE, 0));
    return parse_jump_inst_operand(stream, ret);
  }
  case TOKEN_CALL:
    *ret = presult_instruction(stream->used, INST_CALL(0));
    ++stream->used;
    if (stream->used >= stream->available)
      return PERR_EXPECTED_OPERAND;
    return parse_word_label_or_relative(stream, ret);
  case TOKEN_CALL_STACK:
    *ret = presult_instruction(stream->used, INST_CALL_STACK);
    break;
  case TOKEN_RET:
    *ret = presult_instruction(stream->used, INST_RET);
    break;
  case TOKEN_SYMBOL: {
    size_t label_size = strcspn(token.str, ":");
    if (label_size == token.str_size)
      return PERR_UNKNOWN_OPERATOR;
    else if (label_size != token.str_size - 1)
      return PERR_EXPECTED_LABEL;
    *ret = presult_label(stream->used, token.str, label_size, 0);
    break;
  }
  case TOKEN_STAR:
  default:
    return PERR_UNKNOWN_OPERATOR;
  }
  return perr;
}

perr_t process_presults(presult_t *results, size_t res_count,
                        size_t *result_reached, prog_t **program_ptr)
{
  assert(result_reached && "process_presults: result_reached is NULL?!");
  *result_reached     = 0;
  label_t start_label = {0};

  darr_t label_registry = {0};
  darr_init(&label_registry, sizeof(label_t));
  word inst_count = 0;
  for (size_t i = 0; i < res_count; ++i)
  {
    presult_t res = results[i];
    switch (res.type)
    {
    case PRES_LABEL: {
      label_t label = {.name      = res.label.name,
                       .name_size = res.label.size,
                       .addr      = inst_count};
      darr_append_bytes(&label_registry, (byte *)&label, sizeof(label));
      break;
    }
    case PRES_RELATIVE_ADDRESS: {
      s_word offset = res.address;
      if (offset < 0 && ((word)(-offset)) > inst_count)
      {
        free(label_registry.data);
        *result_reached = i;
        return PERR_INVALID_RELATIVE_ADDRESS;
      }
      results[i].instruction.operand.as_word = ((s_word)inst_count) + offset;
      inst_count++;
      break;
    }
    case PRES_GLOBAL_LABEL: {
      start_label = (label_t){.name      = res.label.name,
                              .name_size = res.label.size,
                              .addr      = (word)inst_count};
      break;
    }
    case PRES_LABEL_ADDRESS:
    case PRES_COMPLETE_RESULT:
      inst_count++;
      break;
    default:
      break;
    }
  }

  darr_t instr_darr = {0};
  darr_init(&instr_darr, sizeof(inst_t));

  prog_header_t header = {0};
  if (start_label.name_size > 0)
  {
    label_t label = search_labels((label_t *)label_registry.data,
                                  label_registry.used / sizeof(label_t),
                                  start_label.name, start_label.name_size);
    if (!label.name)
    {
      free(instr_darr.data);
      free(label_registry.data);
      return PERR_UNKNOWN_LABEL;
    }
    header.start_address = label.addr;
  }

  for (size_t i = 0; i < res_count; ++i)
  {
    presult_t res = results[i];
    switch (res.type)
    {
    case PRES_LABEL_ADDRESS: {
      inst_t inst   = {0};
      label_t label = search_labels((label_t *)label_registry.data,
                                    label_registry.used / sizeof(label_t),
                                    res.label.name, res.label.size);

      if (!label.name)
      {
        free(instr_darr.data);
        free(label_registry.data);
        *result_reached = i;
        return PERR_UNKNOWN_LABEL;
      }

      inst.opcode  = res.instruction.opcode;
      inst.operand = DWORD(label.addr);
      darr_append_bytes(&instr_darr, (byte *)&inst, sizeof(inst));
      break;
    }
    case PRES_RELATIVE_ADDRESS:
    case PRES_COMPLETE_RESULT: {
      darr_append_bytes(&instr_darr, (byte *)&res.instruction,
                        sizeof(res.instruction));
    }
    case PRES_GLOBAL_LABEL:
    case PRES_LABEL:
      break;
    }
  }

  free(label_registry.data);
  prog_t *program =
      malloc(sizeof(**program_ptr) + (sizeof(inst_t) * inst_count));
  program->header = header;
  program->count  = inst_count;
  memcpy(program->instructions, instr_darr.data, instr_darr.used);
  free(instr_darr.data);
  *program_ptr = program;
  return PERR_OK;
}

perr_t parse_stream(token_stream_t *stream, prog_t **program_ptr)
{
  // Preprocessor
  perr_t perr = preprocessor(stream);
  if (perr)
    return perr;
  darr_t presults = {0};
  darr_init(&presults, sizeof(presult_t));
  while (stream->used < stream->available)
  {
    presult_t pres = {0};
    perr_t err     = parse_next(stream, &pres);
    if (err)
    {
      presults_free((presult_t *)presults.data,
                    presults.used / sizeof(presult_t));
      free(presults.data);
      return err;
    }
    darr_append_bytes(&presults, (byte *)&pres, sizeof(presult_t));
    ++stream->used;
  }

  presults.available = presults.used / sizeof(presult_t);
  presults.used      = 0;

#if VERBOSE >= 2
  printf("[%sPARSER%s]: %lu tokens -> %lu parse units\n", TERM_YELLOW,
         TERM_RESET, stream->available, presults.available);
  for (size_t i = 0; i < presults.available; ++i)
  {
    presult_t pres = DARR_AT(presult_t, presults.data, i);
    switch (pres.type)
    {
    case PRES_LABEL:
      printf("\tLABEL: label=%s\n", pres.label.name);
      break;
    case PRES_LABEL_ADDRESS:
      printf("\tLABEL_CALL: label=%s, inst=", pres.label.name);
      inst_print(pres.instruction, stdout);
      printf("\n");
      break;
    case PRES_RELATIVE_ADDRESS:
      printf("\tRELATIVE_CALL: addr=%ld, inst=", pres.address);
      inst_print(pres.instruction, stdout);
      printf("\n");
      break;
    case PRES_GLOBAL_LABEL:
      printf("\tSET_GLOBAL_START: name=%s\n", pres.label.name);
      break;
    case PRES_COMPLETE_RESULT:
      printf("\tCOMPLETE: inst=");
      inst_print(pres.instruction, stdout);
      printf("\n");
      break;
    }
  }
#endif

  size_t results_processed = 0;
  perr = process_presults((presult_t *)presults.data, presults.available,
                          &results_processed, program_ptr);
  if (results_processed != presults.available)
  {
    presult_t pres = DARR_AT(presult_t, presults.data, results_processed);
    stream->used   = pres.stream_index;
  }
  presults_free((presult_t *)presults.data, presults.available);
  free(presults.data);
  return perr;
}
