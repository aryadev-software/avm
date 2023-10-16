/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Implementation of bytecode for instructions
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "./inst.h"

const char *opcode_as_cstr(opcode_t code)
{
  switch (code)
  {
  case OP_NOOP:
    return "NOOP";
    break;
  case OP_PUSH_BYTE:
    return "PUSH_BYTE";
    break;
  case OP_PUSH_WORD:
    return "PUSH_WORD";
    break;
  case OP_PUSH_FLOAT:
    return "PUSH_FLOAT";
    break;
  case OP_PUSH_BYTE_REGISTER:
    return "PUSH_BYTE_REGISTER";
    break;
  case OP_PUSH_WORD_REGISTER:
    return "PUSH_WORD_REGISTER";
    break;
  case OP_PUSH_FLOAT_REGISTER:
    return "PUSH_FLOAT_REGISTER";
    break;
  case OP_POP_BYTE:
    return "POP_BYTE";
    break;
  case OP_POP_WORD:
    return "POP_WORD";
    break;
  case OP_POP_FLOAT:
    return "POP_FLOAT";
    break;
  case OP_MOV_BYTE:
    return "MOV_BYTE";
    break;
  case OP_MOV_WORD:
    return "MOV_WORD";
    break;
  case OP_MOV_FLOAT:
    return "MOV_FLOAT";
    break;
  case OP_HALT:
    return "HALT";
    break;
  }
  return "";
}

const char *opcode_type_as_cstr(opcode_type_t type)
{
  switch (type)
  {
  case OP_TYPE_PUSH:
    return "TYPE_PUSH";
  case OP_TYPE_PUSH_REGISTER:
    return "TYPE_PUSH_REGISTER";
  case OP_TYPE_POP:
    return "TYPE_POP";
  case OP_TYPE_MOV:
    return "TYPE_MOV";
  case OP_TYPE_HALT:
    return "TYPE_HALT";
  }
  return "";
}

void data_print(data_t datum, data_type_t type, FILE *fp)
{
  switch (type)
  {
  case DATA_TYPE_NIL:
    break;
  case DATA_TYPE_BYTE:
    fprintf(fp, "%X", datum.as_byte);
    break;
  case DATA_TYPE_WORD:
    fprintf(fp, "%lX", datum.as_word);
    break;
  case DATA_TYPE_FLOAT:
    fprintf(fp, "%f", datum.as_float);
    break;
  }
}

data_type_t get_opcode_data_type(opcode_t opcode)
{
  data_type_t type = DATA_TYPE_NIL;
  if (OPCODE_IS_TYPE(opcode, OP_TYPE_PUSH))
    type = (data_type_t)opcode;
  else if (OPCODE_IS_TYPE(opcode, OP_TYPE_PUSH_REGISTER))
    type = opcode >> 1;
  else if (OPCODE_IS_TYPE(opcode, OP_TYPE_POP))
    type = opcode >> 2;
  else if (OPCODE_IS_TYPE(opcode, OP_TYPE_MOV))
    type = opcode >> 3;
  return type;
}

void inst_print(inst_t instruction, FILE *fp)
{
  fprintf(fp, "(%s", opcode_as_cstr(instruction.opcode));
  if (OPCODE_IS_TYPE(instruction.opcode, OP_TYPE_PUSH))
  {
    data_type_t type = get_opcode_data_type(instruction.opcode);
    fprintf(fp, ", datum=");
    data_print(instruction.operand, type, fp);
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_TYPE_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_TYPE_MOV))
  {
    fprintf(fp, ", reg=");
    data_print(instruction.operand, DATA_TYPE_BYTE, fp);
  }
  fprintf(fp, ")");
}

size_t inst_bytecode_size(inst_t inst)
{
  size_t size = 1; // for opcode
  if (OPCODE_IS_TYPE(inst.opcode, OP_TYPE_PUSH))
  {
    if (inst.opcode == OP_PUSH_BYTE)
      ++size;
    else if (inst.opcode == OP_PUSH_WORD)
      size += sizeof(word);
    else if (inst.opcode == OP_PUSH_FLOAT)
      size += sizeof(f64);
  }
  else if (OPCODE_IS_TYPE(inst.opcode, OP_TYPE_PUSH_REGISTER))
    ++size;
  else if (OPCODE_IS_TYPE(inst.opcode, OP_TYPE_POP))
  {
    // No operand or register so leave as is
  }
  else if (OPCODE_IS_TYPE(inst.opcode, OP_TYPE_MOV))
  {
    if (inst.opcode == OP_MOV_BYTE)
      ++size;
    else if (inst.opcode == OP_MOV_WORD)
      size += sizeof(word);
    else if (inst.opcode == OP_MOV_FLOAT)
      size += sizeof(f64);
    // For the register
    ++size;
  }
  return size;
}

void inst_write_bytecode(inst_t inst, darr_t *darr)
{
  // Append opcode
  darr_append_byte(darr, inst.opcode);
  // Then append 0 or more operands
  data_type_t to_append = DATA_TYPE_NIL;
  if (OPCODE_IS_TYPE(inst.opcode, OP_TYPE_PUSH))
    to_append = (data_type_t)inst.opcode;
  else if (OPCODE_IS_TYPE(inst.opcode, OP_TYPE_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(inst.opcode, OP_TYPE_MOV))
    to_append = DATA_TYPE_BYTE;

  switch (to_append)
  {
  case DATA_TYPE_NIL:
    break;
  case DATA_TYPE_BYTE:
    darr_append_byte(darr, inst.operand.as_byte);
    break;
  case DATA_TYPE_WORD:
    darr_append_bytes(darr, (byte *)&inst.operand.as_word, sizeof(word));
    break;
  case DATA_TYPE_FLOAT:
    darr_append_bytes(darr, (byte *)&inst.operand.as_float, sizeof(f64));
    break;
  }
}

data_t read_type_from_darr(darr_t *darr, data_type_t type)
{
  switch (type)
  {
  case DATA_TYPE_NIL:
    break;
  case DATA_TYPE_BYTE:
    if (darr->used >= darr->available)
      // TODO: Error (darr has no space left)
      return DBYTE(0);
    return DBYTE(darr->data[darr->used++]);
    break;
  case DATA_TYPE_WORD:
    if (darr->used + sizeof(word) >= darr->available)
      // TODO: Error (darr has no space left)
      return DWORD(0);
    word w = 0;
    memcpy(&w, darr->data + darr->used, sizeof(w));
    darr->used += sizeof(w);
    return DWORD(w);
    break;
  case DATA_TYPE_FLOAT:
    if (darr->used + sizeof(word) >= darr->available)
      // TODO: Error (darr has no space left)
      return DWORD(0);
    f64 f = 0;
    memcpy(&f, darr->data + darr->used, sizeof(f));
    darr->used += sizeof(f);
    return DFLOAT(f);
    break;
  }
  // TODO: Error (unrecognised type)
  return DBYTE(0);
}

inst_t inst_read_bytecode(darr_t *darr)
{
  if (darr->used >= darr->available)
    return (inst_t){0};
  inst_t inst     = {0};
  opcode_t opcode = darr->data[darr->used++];
  if (opcode > OP_HALT)
    // Translate to NOOP
    return inst;
  // Read operands
  if (OPCODE_IS_TYPE(opcode, OP_TYPE_PUSH))
    inst.operand = read_type_from_darr(darr, get_opcode_data_type(opcode));
  // Read register (as a byte)
  else if (OPCODE_IS_TYPE(opcode, OP_TYPE_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(opcode, OP_TYPE_MOV))
    inst.operand = read_type_from_darr(darr, DATA_TYPE_BYTE);
  // Otherwise opcode doesn't take operands

  inst.opcode = opcode;

  return inst;
}
