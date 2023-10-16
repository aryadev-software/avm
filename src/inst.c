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
#include <string.h>

#include "./inst.h"

data_type_t get_opcode_data_type(opcode_t opcode)
{
  data_type_t type = DATA_TYPE_NIL;
  if (OPCODE_IS_TYPE(opcode, OP_TYPE_PUSH))
    type = opcode >> 1;
  else if (OPCODE_IS_TYPE(opcode, OP_TYPE_PUSH_REGISTER))
    type = opcode >> 2;
  else if (OPCODE_IS_TYPE(opcode, OP_TYPE_POP))
    type = opcode >> 3;
  else if (OPCODE_IS_TYPE(opcode, OP_TYPE_MOV))
    type = opcode >> 4;
  return type;
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
  if (OPCODE_IS_TYPE(opcode, OP_TYPE_PUSH_REGISTER) ||
      OPCODE_IS_TYPE(opcode, OP_TYPE_MOV))
    inst.operand = read_type_from_darr(darr, DATA_TYPE_BYTE);
  // Otherwise opcode doesn't take operands

  inst.opcode = opcode;

  return inst;
}
