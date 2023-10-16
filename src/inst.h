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

#include <stdlib.h>

#include "./base.h"
#include "./darr.h"

typedef enum
{
  OP_NOOP = 0,

  // 0b0001
  OP_PUSH_BYTE  = 0b00000001,
  OP_PUSH_WORD  = 0b00000101,
  OP_PUSH_FLOAT = 0b00001001,
  // 0b0010
  OP_PUSH_BYTE_REGISTER  = 0b00000010,
  OP_PUSH_WORD_REGISTER  = 0b00000110,
  OP_PUSH_FLOAT_REGISTER = 0b00001010,
  // 0b0100
  OP_POP_BYTE  = 0b00000100,
  OP_POP_WORD  = 0b00001100,
  OP_POP_FLOAT = 0b00010100,
  // 0b1000
  OP_MOV_BYTE  = 0b00001000,
  OP_MOV_WORD  = 0b00011000,
  OP_MOV_FLOAT = 0b00101000,

  OP_HALT = 0b10000000, // top of the byte is a HALT
} opcode_t;

// Masks and values to check if an opcode is of a type
typedef enum
{
  OP_TYPE_PUSH          = 0b00000001,
  OP_TYPE_PUSH_REGISTER = 0b00000010,
  OP_TYPE_POP           = 0b00000100,
  OP_TYPE_MOV           = 0b00001000,
  OP_TYPE_HALT          = 0b10000000,
} opcode_type_t;

#define OPCODE_IS_TYPE(OPCODE, OP_TYPE) (((OPCODE) & (OP_TYPE)) == (OP_TYPE))

typedef struct
{
  opcode_t opcode;
  data_t operand;
} inst_t;

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

#define INST_BMOV(REG) ((inst_t){.opcode = OP_MOV_BYTE, .operand = DWORD(REG)})

#define INST_WMOV(REG) ((inst_t){.opcode = OP_MOV_WORD, .operand = DWORD(REG)})

#define INST_FMOV(REG) ((inst_t){.opcode = OP_MOV_FLOAT, .operand = DWORD(REG)})

#define INST_BPOP ((inst_t){.opcode = OP_POP_BYTE})

#define INST_WPOP ((inst_t){.opcode = OP_POP_WORD})

#define INST_FPOP ((inst_t){.opcode = OP_POP_FLOAT})

#define INST_BPUSH_REG(REG) \
  ((inst_t){.opcode = OP_PUSH_BYTE_REGISTER, .operand = DWORD(REG)})

#define INST_WPUSH_REG(REG) \
  ((inst_t){.opcode = OP_PUSH_WORD_REGISTER, .operand = DWORD(REG)})

#define INST_FPUSH_REG(REG) \
  ((inst_t){.opcode = OP_PUSH_FLOAT_REGISTER, .operand = DWORD(REG)})

#endif
