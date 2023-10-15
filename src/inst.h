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

#include "./base.h"

typedef enum
{
  OP_NOOP = 0,

  OP_PUSH_BYTE  = 0b0001,
  OP_PUSH_WORD  = 0b0101,
  OP_PUSH_FLOAT = 0b1001,

  OP_HALT,
} opcode_t;

#define OPCODE_IS_PUSH(OPCODE) (((OPCODE)&1) == 1)

typedef struct
{
  opcode_t opcode;
  data_t operand;
} inst_t;

#define INST_BPUSH(BYTE) \
  ((inst_t){.opcode = OP_PUSH_BYTE, .operand = DBYTE(BYTE)})

#define INST_WPUSH(WORD) \
  ((inst_t){.opcode = OP_PUSH_WORD, .operand = DWORD(WORD)})

#define INST_FPUSH(FLOAT) \
  ((inst_t){.opcode = OP_PUSH_FLOAT, .operand = DFLOAT(FLOAT)})

#endif
