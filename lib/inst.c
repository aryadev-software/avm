/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Implementation of bytecode for instructions
 */

#include "./inst.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

const char *opcode_as_cstr(opcode_t code)
{
  switch (code)
  {
  case OP_NOOP:
    return "NOOP";
  case OP_PUSH_BYTE:
    return "PUSH_BYTE";
  case OP_PUSH_WORD:
    return "PUSH_WORD";
  case OP_PUSH_HWORD:
    return "PUSH_HWORD";
  case OP_PUSH_REGISTER_BYTE:
    return "PUSH_REGISTER_BYTE";
  case OP_PUSH_REGISTER_WORD:
    return "PUSH_REGISTER_WORD";
  case OP_PUSH_REGISTER_HWORD:
    return "PUSH_REGISTER_HWORD";
  case OP_POP_BYTE:
    return "POP_BYTE";
  case OP_POP_WORD:
    return "POP_WORD";
  case OP_POP_HWORD:
    return "POP_HWORD";
  case OP_MOV_BYTE:
    return "MOV_BYTE";
  case OP_MOV_WORD:
    return "MOV_WORD";
  case OP_MOV_HWORD:
    return "MOV_HWORD";
  case OP_DUP_BYTE:
    return "DUP_BYTE";
  case OP_DUP_HWORD:
    return "DUP_HWORD";
  case OP_DUP_WORD:
    return "DUP_WORD";
  case OP_MALLOC_BYTE:
    return "MALLOC_BYTE";
  case OP_MALLOC_HWORD:
    return "MALLOC_HWORD";
  case OP_MALLOC_WORD:
    return "MALLOC_WORD";
  case OP_MALLOC_STACK_BYTE:
    return "MALLOC_STACK_BYTE";
  case OP_MALLOC_STACK_HWORD:
    return "MALLOC_STACK_HWORD";
  case OP_MALLOC_STACK_WORD:
    return "MALLOC_STACK_WORD";
  case OP_MSET_BYTE:
    return "MSET_BYTE";
  case OP_MSET_HWORD:
    return "MSET_HWORD";
  case OP_MSET_WORD:
    return "MSET_WORD";
  case OP_MSET_STACK_BYTE:
    return "MSET_STACK_BYTE";
  case OP_MSET_STACK_HWORD:
    return "MSET_STACK_HWORD";
  case OP_MSET_STACK_WORD:
    return "MSET_STACK_WORD";
  case OP_MGET_BYTE:
    return "MGET_BYTE";
  case OP_MGET_HWORD:
    return "MGET_HWORD";
  case OP_MGET_WORD:
    return "MGET_WORD";
  case OP_MGET_STACK_BYTE:
    return "MGET_STACK_BYTE";
  case OP_MGET_STACK_HWORD:
    return "MGET_STACK_HWORD";
  case OP_MGET_STACK_WORD:
    return "MGET_STACK_WORD";
  case OP_MDELETE:
    return "MDELETE";
  case OP_MSIZE:
    return "MDELETE";
  case OP_NOT_BYTE:
    return "NOT_BYTE";
  case OP_NOT_HWORD:
    return "NOT_HWORD";
  case OP_NOT_WORD:
    return "NOT_WORD";
  case OP_OR_BYTE:
    return "OR_BYTE";
  case OP_OR_HWORD:
    return "OR_HWORD";
  case OP_OR_WORD:
    return "OR_WORD";
  case OP_AND_BYTE:
    return "AND_BYTE";
  case OP_AND_HWORD:
    return "AND_HWORD";
  case OP_AND_WORD:
    return "AND_WORD";
  case OP_XOR_BYTE:
    return "XOR_BYTE";
  case OP_XOR_HWORD:
    return "XOR_HWORD";
  case OP_XOR_WORD:
    return "XOR_WORD";
  case OP_EQ_BYTE:
    return "EQ_BYTE";
  case OP_EQ_HWORD:
    return "EQ_HWORD";
  case OP_EQ_WORD:
    return "EQ_WORD";
  case OP_LT_BYTE:
    return "LT_BYTE";
  case OP_LT_CHAR:
    return "LT_CHAR";
  case OP_LT_HWORD:
    return "LT_HWORD";
  case OP_LT_INT:
    return "LT_INT";
  case OP_LT_LONG:
    return "LT_LONG";
  case OP_LT_WORD:
    return "LT_WORD";
  case OP_LTE_BYTE:
    return "LTE_BYTE";
  case OP_LTE_CHAR:
    return "LTE_CHAR";
  case OP_LTE_HWORD:
    return "LTE_HWORD";
  case OP_LTE_INT:
    return "LTE_INT";
  case OP_LTE_LONG:
    return "LTE_LONG";
  case OP_LTE_WORD:
    return "LTE_WORD";
  case OP_GT_BYTE:
    return "GT_BYTE";
  case OP_GT_CHAR:
    return "GT_CHAR";
  case OP_GT_HWORD:
    return "GT_HWORD";
  case OP_GT_INT:
    return "GT_INT";
  case OP_GT_LONG:
    return "GT_LONG";
  case OP_GT_WORD:
    return "GT_WORD";
  case OP_GTE_BYTE:
    return "GTE_BYTE";
  case OP_GTE_CHAR:
    return "GTE_CHAR";
  case OP_GTE_HWORD:
    return "GTE_HWORD";
  case OP_GTE_INT:
    return "GTE_INT";
  case OP_GTE_LONG:
    return "GTE_LONG";
  case OP_GTE_WORD:
    return "GTE_WORD";
  case OP_PLUS_BYTE:
    return "PLUS_BYTE";
  case OP_PLUS_HWORD:
    return "PLUS_HWORD";
  case OP_PLUS_WORD:
    return "PLUS_WORD";
  case OP_SUB_BYTE:
    return "SUB_BYTE";
  case OP_SUB_HWORD:
    return "SUB_HWORD";
  case OP_SUB_WORD:
    return "SUB_WORD";
  case OP_MULT_BYTE:
    return "MULT_BYTE";
  case OP_MULT_HWORD:
    return "MULT_HWORD";
  case OP_MULT_WORD:
    return "MULT_WORD";
  case OP_JUMP_ABS:
    return "JUMP_ABS";
  case OP_JUMP_STACK:
    return "JUMP_STACK";
  case OP_JUMP_IF_BYTE:
    return "JUMP_IF_BYTE";
  case OP_JUMP_IF_HWORD:
    return "JUMP_IF_HWORD";
  case OP_JUMP_IF_WORD:
    return "JUMP_IF_WORD";
  case OP_CALL:
    return "CALL";
  case OP_CALL_STACK:
    return "CALL_STACK";
  case OP_RET:
    return "RET";
  case OP_PRINT_CHAR:
    return "PRINT_CHAR";
  case OP_PRINT_BYTE:
    return "PRINT_BYTE";
  case OP_PRINT_INT:
    return "PRINT_INT";
  case OP_PRINT_HWORD:
    return "PRINT_HWORD";
  case OP_PRINT_LONG:
    return "PRINT_LONG";
  case OP_PRINT_WORD:
    return "PRINT_WORD";
  case OP_HALT:
    return "HALT";
  case NUMBER_OF_OPCODES:
    return "";
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
    fprintf(fp, "%X", datum.as_hword);
    break;
  case DATA_TYPE_WORD:
    fprintf(fp, "%lX", datum.as_word);
    break;
  }
}

void inst_print(inst_t instruction, FILE *fp)
{
  static_assert(NUMBER_OF_OPCODES == 98, "inst_bytecode_size: Out of date");
  fprintf(fp, "%s(", opcode_as_cstr(instruction.opcode));
  if (OPCODE_IS_TYPE(instruction.opcode, OP_PUSH))
  {
    data_type_t type = (data_type_t)instruction.opcode;
    fprintf(fp, "datum=0x");
    data_print(instruction.operand, type, fp);
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MOV))
  {
    fprintf(fp, "reg=0x");
    data_print(instruction.operand, DATA_TYPE_BYTE, fp);
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_DUP) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MALLOC) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MSET) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MGET))
  {
    fprintf(fp, "n=%lu", instruction.operand.as_word);
  }
  else if (instruction.opcode == OP_JUMP_ABS ||
           OPCODE_IS_TYPE(instruction.opcode, OP_JUMP_IF) ||
           instruction.opcode == OP_CALL)
  {
    fprintf(fp, "address=0x");
    data_print(instruction.operand, DATA_TYPE_WORD, fp);
  }
  fprintf(fp, ")");
}

size_t inst_bytecode_size(inst_t inst)
{
  static_assert(NUMBER_OF_OPCODES == 98, "inst_bytecode_size: Out of date");
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
           OPCODE_IS_TYPE(inst.opcode, OP_DUP) ||
           OPCODE_IS_TYPE(inst.opcode, OP_MALLOC) ||
           OPCODE_IS_TYPE(inst.opcode, OP_MSET) ||
           OPCODE_IS_TYPE(inst.opcode, OP_MGET) || inst.opcode == OP_JUMP_ABS ||
           OPCODE_IS_TYPE(inst.opcode, OP_JUMP_IF) || inst.opcode == OP_CALL)
    size += WORD_SIZE;
  return size;
}

void inst_write_bytecode(inst_t inst, darr_t *darr)
{
  static_assert(NUMBER_OF_OPCODES == 98, "inst_write_bytecode: Out of date");
  // Append opcode
  darr_append_byte(darr, inst.opcode);
  // Then append 0 or more operands
  data_type_t to_append = DATA_TYPE_NIL;
  if (OPCODE_IS_TYPE(inst.opcode, OP_PUSH))
    to_append = (data_type_t)inst.opcode;
  else if (OPCODE_IS_TYPE(inst.opcode, OP_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(inst.opcode, OP_MOV) ||
           OPCODE_IS_TYPE(inst.opcode, OP_DUP) ||
           OPCODE_IS_TYPE(inst.opcode, OP_MALLOC) ||
           OPCODE_IS_TYPE(inst.opcode, OP_MSET) ||
           OPCODE_IS_TYPE(inst.opcode, OP_MGET) || inst.opcode == OP_JUMP_ABS ||
           OPCODE_IS_TYPE(inst.opcode, OP_JUMP_IF) || inst.opcode == OP_CALL)
    to_append = DATA_TYPE_WORD;

  switch (to_append)
  {
  case DATA_TYPE_NIL:
    break;
  case DATA_TYPE_BYTE:
    darr_append_byte(darr, inst.operand.as_byte);
    break;
  case DATA_TYPE_HWORD:
    darr_ensure_capacity(darr, HWORD_SIZE);
    convert_hword_to_bytes(inst.operand.as_hword, darr->data + darr->used);
    darr->used += HWORD_SIZE;
    break;
  case DATA_TYPE_WORD:
    darr_ensure_capacity(darr, WORD_SIZE);
    convert_word_to_bytes(inst.operand.as_word, darr->data + darr->used);
    darr->used += WORD_SIZE;
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
    if (darr->used > darr->available)
      // TODO: Error (darr has no space left)
      return DBYTE(0);
    return DBYTE(darr->data[darr->used++]);
    break;
  case DATA_TYPE_HWORD:
    if (darr->used + HWORD_SIZE > darr->available)
      // TODO: Error (darr has no space left)
      return DWORD(0);
    hword u = convert_bytes_to_hword(darr->data + darr->used);
    darr->used += HWORD_SIZE;
    return DHWORD(u);
    break;
  case DATA_TYPE_WORD:
    if (darr->used + WORD_SIZE > darr->available)
      // TODO: Error (darr has no space left)
      return DWORD(0);
    word w = convert_bytes_to_word(darr->data + darr->used);
    darr->used += WORD_SIZE;
    return DWORD(w);
    break;
  }
  // TODO: Error (unrecognised type)
  return DBYTE(0);
}

inst_t inst_read_bytecode(darr_t *darr)
{
  static_assert(NUMBER_OF_OPCODES == 98, "inst_read_bytecode: Out of date");
  if (darr->used >= darr->available)
    return (inst_t){0};
  inst_t inst     = {0};
  opcode_t opcode = darr->data[darr->used++];
  if (opcode > OP_HALT || opcode == NUMBER_OF_OPCODES || opcode < OP_NOOP)
    return INST_NOOP;
  // Read operands
  if (OPCODE_IS_TYPE(opcode, OP_PUSH))
    inst.operand = read_type_from_darr(darr, (data_type_t)opcode);
  // Read register (as a byte)
  else if (OPCODE_IS_TYPE(opcode, OP_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(opcode, OP_MOV) || OPCODE_IS_TYPE(opcode, OP_DUP) ||
           OPCODE_IS_TYPE(opcode, OP_MALLOC) ||
           OPCODE_IS_TYPE(opcode, OP_MSET) || OPCODE_IS_TYPE(opcode, OP_MGET) ||
           opcode == OP_JUMP_ABS || OPCODE_IS_TYPE(opcode, OP_JUMP_IF) ||
           opcode == OP_CALL)
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

inst_t *insts_read_bytecode_file(FILE *fp, size_t *ret)
{
  darr_t darr          = darr_read_file(fp);
  inst_t *instructions = insts_read_bytecode(&darr, ret);
  free(darr.data);
  return instructions;
}

void insts_write_bytecode_file(inst_t *instructions, size_t size, FILE *fp)
{
  darr_t darr = {0};
  darr_init(&darr, 0);
  insts_write_bytecode(instructions, size, &darr);
  darr_write_file(&darr, fp);
  free(darr.data);
}

void prog_header_write_bytecode(prog_header_t header, darr_t *buffer)
{
  word start = word_htobc(header.start_address);
  darr_append_bytes(buffer, (byte *)&start, sizeof(start));
  word count = word_htobc(header.count);
  darr_append_bytes(buffer, (byte *)&count, sizeof(count));
}

void prog_write_bytecode(prog_t *program, darr_t *buffer)
{
  // Write program header
  prog_header_write_bytecode(program->header, buffer);
  // Write instructions
  insts_write_bytecode(program->instructions, program->header.count, buffer);
}

void prog_append_bytecode(prog_t *program, darr_t *buffer)
{
  insts_write_bytecode(program->instructions, program->header.count, buffer);
}

prog_header_t prog_header_read_bytecode(darr_t *buffer)
{
  prog_header_t header = {0};
  header.start_address = convert_bytes_to_word(buffer->data + buffer->used);
  buffer->used += sizeof(header.start_address);
  header.count = convert_bytes_to_word(buffer->data + buffer->used);
  buffer->used += sizeof(header.count);
  return header;
}

prog_t *prog_read_bytecode(darr_t *buffer)
{
  // TODO: Error (not enough space for program header)
  if ((buffer->available - buffer->used) < sizeof(prog_header_t))
    return NULL;
  // Read program header
  prog_header_t header = prog_header_read_bytecode(buffer);
  // TODO: Error (not enough space for program instruction count)
  if ((buffer->available - buffer->used) < WORD_SIZE)
    return NULL;

  prog_t *program = malloc(sizeof(*program) + (sizeof(inst_t) * header.count));
  size_t i;
  for (i = 0; i < header.count && (buffer->used < buffer->available); ++i)
    program->instructions[i] = inst_read_bytecode(buffer);

  // TODO: Error (Expected more instructions)
  if (i < header.count - 1)
  {
    free(program);
    return NULL;
  }

  program->header = header;

  return program;
}

void prog_write_file(prog_t *program, FILE *fp)
{
  darr_t bytecode = {0};
  prog_write_bytecode(program, &bytecode);
  fwrite(bytecode.data, bytecode.used, 1, fp);
  free(bytecode.data);
}

prog_t *prog_read_file(FILE *fp)
{
  darr_t buffer = darr_read_file(fp);
  prog_t *p     = prog_read_bytecode(&buffer);
  free(buffer.data);
  return p;
}
