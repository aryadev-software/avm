/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Instructions and opcodes
 */

#ifndef INST_H
#define INST_H

#include <lib/darr.h>
#include <lib/prog.h>

#include <stdio.h>
#include <stdlib.h>

const char *opcode_as_cstr(opcode_t);

#define OPCODE_IS_TYPE(OPCODE, OP_TYPE) \
  (((OPCODE) >= OP_TYPE##_BYTE) && ((OPCODE) <= OP_TYPE##_WORD))

#define OPCODE_DATA_TYPE(OPCODE, OP_TYPE)            \
  ((OPCODE) == OP_TYPE##_BYTE      ? DATA_TYPE_BYTE  \
   : ((OPCODE) == OP_TYPE##_HWORD) ? DATA_TYPE_HWORD \
                                   : DATA_TYPE_WORD)

void inst_print(inst_t, FILE *);

size_t inst_bytecode_size(inst_t);
void inst_write_bytecode(inst_t, darr_t *);
void insts_write_bytecode(inst_t *, size_t, darr_t *);
// Here the dynamic array is a preloaded buffer of bytes, where
// darr.available is the number of overall bytes and used is the
// cursor (where we are in the buffer).
inst_t inst_read_bytecode(darr_t *);
inst_t *insts_read_bytecode(darr_t *, size_t *);

void insts_write_bytecode_file(inst_t *, size_t, FILE *);
inst_t *insts_read_bytecode_file(FILE *, size_t *);

// Write the entire program as bytecode
void prog_write_bytecode(prog_t *, darr_t *);
// Only append the instructions as bytecode
void prog_append_bytecode(prog_t *, darr_t *);
// Read an entire program as bytecode
prog_t *prog_read_bytecode(darr_t *);

void prog_write_file(prog_t *, FILE *);
prog_t *prog_read_file(FILE *);

#define INST_NOOP ((inst_t){0})
#define INST_HALT ((inst_t){.opcode = OP_HALT})

#define INST_PUSH(TYPE, OP) \
  ((inst_t){.opcode = OP_PUSH_##TYPE, .operand = D##TYPE(OP)})

#define INST_MOV(TYPE, OP) \
  ((inst_t){.opcode = OP_MOV_##TYPE, .operand = D##TYPE(OP)})

#define INST_POP(TYPE) ((inst_t){.opcode = OP_POP_##TYPE})

#define INST_PUSH_REG(TYPE, REG) \
  ((inst_t){.opcode = OP_PUSH_REGISTER_##TYPE, .operand = D##TYPE(REG)})

#define INST_DUP(TYPE, OP) \
  ((inst_t){.opcode = OP_DUP_##TYPE, .operand = DWORD(OP)})

#define INST_MALLOC(TYPE, OP) \
  ((inst_t){.opcode = OP_MALLOC_##TYPE, .operand = DWORD(OP)})
#define INST_MALLOC_STACK(TYPE) ((inst_t){.opcode = OP_MALLOC_STACK_##TYPE})
#define INST_MSET(TYPE, OP) \
  ((inst_t){.opcode = OP_MSET_##TYPE, .operand = DWORD(OP)})
#define INST_MSET_STACK(TYPE) ((inst_t){.opcode = OP_MSET_STACK_##TYPE})
#define INST_MGET(TYPE, OP) \
  ((inst_t){.opcode = OP_MGET_##TYPE, .operand = DWORD(OP)})
#define INST_MGET_STACK(TYPE) ((inst_t){.opcode = OP_MGET_STACK_##TYPE})
#define INST_MDELETE          ((inst_t){.opcode = OP_MDELETE})
#define INST_MSIZE            ((inst_t){.opcode = OP_MSIZE})

#define INST_NOT(TYPE)  ((inst_t){.opcode = OP_NOT_##TYPE})
#define INST_OR(TYPE)   ((inst_t){.opcode = OP_OR_##TYPE})
#define INST_AND(TYPE)  ((inst_t){.opcode = OP_AND_##TYPE})
#define INST_XOR(TYPE)  ((inst_t){.opcode = OP_XOR_##TYPE})
#define INST_EQ(TYPE)   ((inst_t){.opcode = OP_EQ_##TYPE})
#define INST_LT(TYPE)   ((inst_t){.opcode = OP_LT_##TYPE})
#define INST_LTE(TYPE)  ((inst_t){.opcode = OP_LTE_##TYPE})
#define INST_GT(TYPE)   ((inst_t){.opcode = OP_GT_##TYPE})
#define INST_GTE(TYPE)  ((inst_t){.opcode = OP_GTE_##TYPE})
#define INST_PLUS(TYPE) ((inst_t){.opcode = OP_PLUS_##TYPE})
#define INST_SUB(TYPE)  ((inst_t){.opcode = OP_SUB_##TYPE})
#define INST_MULT(TYPE) ((inst_t){.opcode = OP_MULT_##TYPE})

#define INST_JUMP_ABS(OP) \
  ((inst_t){.opcode = OP_JUMP_ABS, .operand = DWORD(OP)})
#define INST_JUMP_STACK ((inst_t){.opcode = OP_JUMP_STACK})
#define INST_JUMP_IF(TYPE, OP) \
  ((inst_t){.opcode = OP_JUMP_IF_##TYPE, .operand = DWORD(OP)})
#define INST_CALL(OP)   ((inst_t){.opcode = OP_CALL, .operand = DWORD(OP)})
#define INST_CALL_STACK ((inst_t){.opcode = OP_CALL_STACK})
#define INST_RET        ((inst_t){.opcode = OP_RET})

#define INST_PRINT(TYPE) ((inst_t){.opcode = OP_PRINT_##TYPE})
#endif
