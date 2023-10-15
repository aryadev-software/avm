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
  OP_PUSH_BYTE = 1,
  OP_PUSH_WORD,
  OP_PUSH_FLOAT,
} op_t;

typedef struct
{
  op_t opcode;
  data_t operand;
} inst_t;

#define INST_BPUSH(BYTE) \
  ((inst_t){.opcode = OP_PUSH_BYTE, .operand = DBYTE(BYTE)})

#define INST_WPUSH(WORD) \
  ((inst_t){.opcode = OP_PUSH_WORD, .operand = DWORD(WORD)})

#define INST_FPUSH(FLOAT) \
  ((inst_t){.opcode = OP_PUSH_FLOAT, .operand = DFLOAT(FLOAT)})

#endif
