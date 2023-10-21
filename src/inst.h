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

#include <stdio.h>
#include <stdlib.h>

#include "./base.h"
#include "./darr.h"

typedef enum
{
  OP_NOOP = 0,

  OP_PUSH_BYTE,
  OP_PUSH_HWORD,
  OP_PUSH_WORD,

  OP_PUSH_REGISTER_BYTE,
  OP_PUSH_REGISTER_HWORD,
  OP_PUSH_REGISTER_WORD,

  OP_POP_BYTE,
  OP_POP_HWORD,
  OP_POP_WORD,

  OP_MOV_BYTE,
  OP_MOV_HWORD,
  OP_MOV_WORD,

  OP_HALT = 0b11111111, // top of the byte is a HALT
} opcode_t;

const char *opcode_as_cstr(opcode_t);

#define OPCODE_IS_TYPE(OPCODE, OP_TYPE) \
  (((OPCODE) >= OP_TYPE##_BYTE) && ((OPCODE) <= OP_TYPE##_WORD))

typedef struct
{
  opcode_t opcode;
  data_t operand;
} inst_t;

void inst_print(inst_t, FILE *);

size_t inst_bytecode_size(inst_t);
void inst_write_bytecode(inst_t, darr_t *);
// Here the dynamic array is a preloaded buffer of bytes, where
// darr.available is the number of overall bytes and used is the
// cursor (where we are in the buffer).
inst_t inst_read_bytecode(darr_t *);

#define INST_BPUSH(BYTE) \
  ((inst_t){.opcode = OP_PUSH_BYTE, .operand = DBYTE(BYTE)})

#define INST_WPUSH(WORD) \
  ((inst_t){.opcode = OP_PUSH_WORD, .operand = DWORD(WORD)})

#define INST_FPUSH(FLOAT) \
  ((inst_t){.opcode = OP_PUSH_FLOAT, .operand = DFLOAT(FLOAT)})

#define INST_BMOV(REG) ((inst_t){.opcode = OP_MOV_BYTE, .operand = DBYTE(REG)})

#define INST_WMOV(REG) ((inst_t){.opcode = OP_MOV_WORD, .operand = DBYTE(REG)})

#define INST_FMOV(REG) ((inst_t){.opcode = OP_MOV_FLOAT, .operand = DBYTE(REG)})

#define INST_BPOP ((inst_t){.opcode = OP_POP_BYTE})

#define INST_WPOP ((inst_t){.opcode = OP_POP_WORD})

#define INST_FPOP ((inst_t){.opcode = OP_POP_FLOAT})

#define INST_BPUSH_REG(REG) \
  ((inst_t){.opcode = OP_PUSH_REGISTER_BYTE, .operand = DBYTE(REG)})

#define INST_WPUSH_REG(REG) \
  ((inst_t){.opcode = OP_PUSH_REGISTER_WORD, .operand = DBYTE(REG)})

#define INST_FPUSH_REG(REG) \
  ((inst_t){.opcode = OP_PUSH_REGISTER_FLOAT, .operand = DBYTE(REG)})

#endif
