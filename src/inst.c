/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Implementation of bytecode for instructions
 */

#include <assert.h>
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
  case OP_PUSH_HWORD:
    return "PUSH_HWORD";
    break;
  case OP_PUSH_REGISTER_BYTE:
    return "PUSH_REGISTER_BYTE";
    break;
  case OP_PUSH_REGISTER_WORD:
    return "PUSH_REGISTER_WORD";
    break;
  case OP_PUSH_REGISTER_HWORD:
    return "PUSH_REGISTER_HWORD";
    break;
  case OP_POP_BYTE:
    return "POP_BYTE";
    break;
  case OP_POP_WORD:
    return "POP_WORD";
    break;
  case OP_POP_HWORD:
    return "POP_HWORD";
    break;
  case OP_MOV_BYTE:
    return "MOV_BYTE";
    break;
  case OP_MOV_WORD:
    return "MOV_WORD";
    break;
  case OP_MOV_HWORD:
    return "MOV_HWORD";
    break;
  case OP_DUP_BYTE:
    return "DUP_BYTE";
    break;
  case OP_DUP_HWORD:
    return "DUP_HWORD";
    break;
  case OP_DUP_WORD:
    return "DUP_WORD";
    break;
  case OP_NOT_BYTE:
    return "NOT_BYTE";
    break;
  case OP_NOT_HWORD:
    return "NOT_HWORD";
    break;
  case OP_NOT_WORD:
    return "NOT_WORD";
    break;
  case OP_OR_BYTE:
    return "OR_BYTE";
    break;
  case OP_OR_HWORD:
    return "OR_HWORD";
    break;
  case OP_OR_WORD:
    return "OR_WORD";
    break;
  case OP_AND_BYTE:
    return "AND_BYTE";
    break;
  case OP_AND_HWORD:
    return "AND_HWORD";
    break;
  case OP_AND_WORD:
    return "AND_WORD";
    break;
  case OP_XOR_BYTE:
    return "XOR_BYTE";
    break;
  case OP_XOR_HWORD:
    return "XOR_HWORD";
    break;
  case OP_XOR_WORD:
    return "XOR_WORD";
    break;
  case OP_EQ_BYTE:
    return "EQ_BYTE";
    break;
  case OP_EQ_HWORD:
    return "EQ_HWORD";
    break;
  case OP_EQ_WORD:
    return "EQ_WORD";
    break;
  case OP_PLUS_BYTE:
    return "PLUS_BYTE";
    break;
  case OP_PLUS_HWORD:
    return "PLUS_HWORD";
    break;
  case OP_PLUS_WORD:
    return "PLUS_WORD";
    break;
  case OP_JUMP_ABS:
    return "JUMP_ABS";
    break;
  case OP_JUMP_STACK:
    return "JUMP_STACK";
    break;
  case OP_JUMP_REGISTER:
    return "JUMP_REGISTER";
    break;
  case OP_PRINT_CHAR:
    return "PRINT_CHAR";
    break;
  case OP_PRINT_BYTE:
    return "PRINT_BYTE";
    break;
  case OP_PRINT_INT:
    return "PRINT_INT";
    break;
  case OP_PRINT_HWORD:
    return "PRINT_HWORD";
    break;
  case OP_PRINT_LONG:
    return "PRINT_LONG";
    break;
  case OP_PRINT_WORD:
    return "PRINT_WORD";
    break;
  case OP_HALT:
    return "HALT";
    break;
  case NUMBER_OF_OPCODES:
    return "";
    break;
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
  case DATA_TYPE_HWORD:
    fprintf(fp, "%d", datum.as_hword);
    break;
  case DATA_TYPE_WORD:
    fprintf(fp, "%lX", datum.as_word);
    break;
  }
}

hword convert_bytes_to_hword(byte *bytes)
{
  hword h = 0;
  memcpy(&h, bytes, HWORD_SIZE);
  return h;
}

void convert_hword_to_bytes(hword w, byte *bytes)
{
  memcpy(bytes, &w, HWORD_SIZE);
}

void convert_word_to_bytes(word w, byte *bytes)
{
  memcpy(bytes, &w, WORD_SIZE);
}

word convert_bytes_to_word(byte *bytes)
{
  word w = 0;
  memcpy(&w, bytes, WORD_SIZE);
  return w;
}

void inst_print(inst_t instruction, FILE *fp)
{
  static_assert(NUMBER_OF_OPCODES == 43, "inst_bytecode_size: Out of date");
  fprintf(fp, "%s(", opcode_as_cstr(instruction.opcode));
  if (OPCODE_IS_TYPE(instruction.opcode, OP_PUSH))
  {
    data_type_t type = (data_type_t)instruction.opcode;
    fprintf(fp, "datum=0x");
    data_print(instruction.operand, type, fp);
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MOV) ||
           instruction.opcode == OP_JUMP_REGISTER)
  {
    fprintf(fp, "reg=0x");
    data_print(instruction.operand, DATA_TYPE_BYTE, fp);
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_DUP))
  {
    fprintf(fp, "n=%lu", instruction.operand.as_word);
  }
  else if (instruction.opcode == OP_JUMP_ABS)
  {
    fprintf(fp, "address=0x");
    data_print(instruction.operand, DATA_TYPE_WORD, fp);
  }
  fprintf(fp, ")");
}

size_t inst_bytecode_size(inst_t inst)
{
  static_assert(NUMBER_OF_OPCODES == 43, "inst_bytecode_size: Out of date");
  size_t size = 1; // for opcode
  if (OPCODE_IS_TYPE(inst.opcode, OP_PUSH))
  {
    if (inst.opcode == OP_PUSH_BYTE)
      ++size;
    else if (inst.opcode == OP_PUSH_HWORD)
      size += HWORD_SIZE;
    else if (inst.opcode == OP_PUSH_WORD)
      size += WORD_SIZE;
  }
  else if (OPCODE_IS_TYPE(inst.opcode, OP_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(inst.opcode, OP_MOV) ||
           inst.opcode == OP_JUMP_REGISTER)
    // Only need a byte for the register
    ++size;
  else if (OPCODE_IS_TYPE(inst.opcode, OP_DUP))
    size += WORD_SIZE;
  else if (inst.opcode == OP_JUMP_ABS)
    size += WORD_SIZE;
  return size;
}

void inst_write_bytecode(inst_t inst, darr_t *darr)
{
  static_assert(NUMBER_OF_OPCODES == 43, "inst_write_bytecode: Out of date");
  // Append opcode
  darr_append_byte(darr, inst.opcode);
  // Then append 0 or more operands
  data_type_t to_append = DATA_TYPE_NIL;
  if (OPCODE_IS_TYPE(inst.opcode, OP_PUSH))
    to_append = (data_type_t)inst.opcode;
  else if (OPCODE_IS_TYPE(inst.opcode, OP_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(inst.opcode, OP_MOV) ||
           inst.opcode == OP_JUMP_REGISTER)
    to_append = DATA_TYPE_BYTE;
  else if (OPCODE_IS_TYPE(inst.opcode, OP_DUP) || inst.opcode == OP_JUMP_ABS)
    to_append = DATA_TYPE_WORD;

  switch (to_append)
  {
  case DATA_TYPE_NIL:
    break;
  case DATA_TYPE_BYTE:
    darr_append_byte(darr, inst.operand.as_byte);
    break;
  case DATA_TYPE_HWORD:
    darr_append_bytes(darr, (byte *)&inst.operand.as_hword, HWORD_SIZE);
    break;
  case DATA_TYPE_WORD:
    darr_append_bytes(darr, (byte *)&inst.operand.as_word, WORD_SIZE);
    break;
  }
}

void insts_write_bytecode(inst_t *insts, size_t size, darr_t *darr)
{
  for (size_t i = 0; i < size; ++i)
    inst_write_bytecode(insts[i], darr);
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
  case DATA_TYPE_HWORD:
    if (darr->used + HWORD_SIZE >= darr->available)
      // TODO: Error (darr has no space left)
      return DWORD(0);
    hword u = 0;
    memcpy(&u, darr->data + darr->used, HWORD_SIZE);
    darr->used += HWORD_SIZE;
    return DHWORD(u);
    break;
  case DATA_TYPE_WORD:
    if (darr->used + WORD_SIZE >= darr->available)
      // TODO: Error (darr has no space left)
      return DWORD(0);
    word w = 0;
    memcpy(&w, darr->data + darr->used, WORD_SIZE);
    darr->used += WORD_SIZE;
    return DWORD(w);
    break;
  }
  // TODO: Error (unrecognised type)
  return DBYTE(0);
}

inst_t inst_read_bytecode(darr_t *darr)
{
  static_assert(NUMBER_OF_OPCODES == 43, "inst_read_bytecode: Out of date");
  if (darr->used >= darr->available)
    return (inst_t){0};
  inst_t inst     = {0};
  opcode_t opcode = darr->data[darr->used++];
  if (opcode > OP_HALT || opcode == NUMBER_OF_OPCODES || opcode < OP_NOOP)
    // Translate to NOOP
    return inst;
  // Read operands
  if (OPCODE_IS_TYPE(opcode, OP_PUSH))
    inst.operand = read_type_from_darr(darr, (data_type_t)opcode);
  // Read register (as a byte)
  else if (OPCODE_IS_TYPE(opcode, OP_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(opcode, OP_MOV) || inst.opcode == OP_JUMP_STACK)
    inst.operand = read_type_from_darr(darr, DATA_TYPE_BYTE);
  else if (OPCODE_IS_TYPE(opcode, OP_DUP) || opcode == OP_JUMP_ABS)
    inst.operand = read_type_from_darr(darr, DATA_TYPE_WORD);
  // Otherwise opcode doesn't take operands

  inst.opcode = opcode;

  return inst;
}

inst_t *insts_read_bytecode(darr_t *bytes, size_t *ret_size)
{
  *ret_size = 0;
  // NOTE: Here we use the darr as a dynamic array of inst_t.
  darr_t instructions = {0};
  darr_init(&instructions, sizeof(inst_t));
  while (bytes->used < bytes->available)
  {
    inst_t instruction = inst_read_bytecode(bytes);
    darr_append_bytes(&instructions, (byte *)&instruction, sizeof(instruction));
  }
  *ret_size = instructions.used / sizeof(inst_t);
  return (inst_t *)instructions.data;
}

void insts_write_bytecode_file(inst_t *instructions, size_t size, FILE *fp)
{
  darr_t darr = {0};
  darr_init(&darr, 0);
  insts_write_bytecode(instructions, size, &darr);
  darr_write_file(&darr, fp);
  free(darr.data);
}

inst_t *insts_read_bytecode_file(FILE *fp, size_t *ret)
{
  darr_t darr          = darr_read_file(fp);
  inst_t *instructions = insts_read_bytecode(&darr, ret);
  free(darr.data);
  return instructions;
}
