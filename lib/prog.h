/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2024-04-14
 * Author: Aryadev Chavali
 * Description: Structures for both instructions and programs for the
 * virtual machine
 */

#ifndef PROG_H
#define PROG_H

#include <lib/base.h>

typedef enum
{
  OP_NOOP = 0,

  // Dealing with data and registers
  OP_PUSH_BYTE,
  OP_PUSH_HWORD,
  OP_PUSH_WORD,

  OP_POP_BYTE,
  OP_POP_HWORD,
  OP_POP_WORD,

  OP_PUSH_REGISTER_BYTE,
  OP_PUSH_REGISTER_HWORD,
  OP_PUSH_REGISTER_WORD,

  OP_MOV_BYTE,
  OP_MOV_HWORD,
  OP_MOV_WORD,

  OP_DUP_BYTE,
  OP_DUP_HWORD,
  OP_DUP_WORD,

  // Dealing with the heap
  OP_MALLOC_BYTE,
  OP_MALLOC_HWORD,
  OP_MALLOC_WORD,

  OP_MALLOC_STACK_BYTE,
  OP_MALLOC_STACK_HWORD,
  OP_MALLOC_STACK_WORD,

  OP_MSET_BYTE,
  OP_MSET_HWORD,
  OP_MSET_WORD,

  OP_MSET_STACK_BYTE,
  OP_MSET_STACK_HWORD,
  OP_MSET_STACK_WORD,

  OP_MGET_BYTE,
  OP_MGET_HWORD,
  OP_MGET_WORD,

  OP_MGET_STACK_BYTE,
  OP_MGET_STACK_HWORD,
  OP_MGET_STACK_WORD,

  OP_MDELETE,
  OP_MSIZE,

  // Boolean operations
  OP_NOT_BYTE,
  OP_NOT_HWORD,
  OP_NOT_WORD,

  OP_OR_BYTE,
  OP_OR_HWORD,
  OP_OR_WORD,

  OP_AND_BYTE,
  OP_AND_HWORD,
  OP_AND_WORD,

  OP_XOR_BYTE,
  OP_XOR_HWORD,
  OP_XOR_WORD,

  OP_EQ_BYTE,
  OP_EQ_HWORD,
  OP_EQ_WORD,

  // Mathematical operations
  OP_LT_BYTE,
  OP_LT_CHAR,
  OP_LT_HWORD,
  OP_LT_INT,
  OP_LT_LONG,
  OP_LT_WORD,

  OP_LTE_BYTE,
  OP_LTE_CHAR,
  OP_LTE_HWORD,
  OP_LTE_INT,
  OP_LTE_LONG,
  OP_LTE_WORD,

  OP_GT_BYTE,
  OP_GT_CHAR,
  OP_GT_HWORD,
  OP_GT_INT,
  OP_GT_LONG,
  OP_GT_WORD,

  OP_GTE_BYTE,
  OP_GTE_CHAR,
  OP_GTE_HWORD,
  OP_GTE_INT,
  OP_GTE_LONG,
  OP_GTE_WORD,

  OP_PLUS_BYTE,
  OP_PLUS_HWORD,
  OP_PLUS_WORD,

  OP_SUB_BYTE,
  OP_SUB_HWORD,
  OP_SUB_WORD,

  OP_MULT_BYTE,
  OP_MULT_HWORD,
  OP_MULT_WORD,

  // Simple I/O
  OP_PRINT_BYTE,
  OP_PRINT_CHAR,
  OP_PRINT_HWORD,
  OP_PRINT_INT,
  OP_PRINT_LONG,
  OP_PRINT_WORD,

  // Program control flow
  OP_JUMP_ABS,
  OP_JUMP_STACK,
  OP_JUMP_IF_BYTE,
  OP_JUMP_IF_HWORD,
  OP_JUMP_IF_WORD,
  // Subroutines
  OP_CALL,
  OP_CALL_STACK,
  OP_RET,

  // Should not be an opcode
  NUMBER_OF_OPCODES,
  OP_HALT = 0b11111111, // top of the byte is a HALT
} opcode_t;

typedef struct
{
  opcode_t opcode;
  data_t operand;
} inst_t;

typedef struct
{
  word start_address;
} prog_header_t;

typedef struct
{
  prog_header_t header;
  word count;
  inst_t instructions[];
} prog_t;

#endif
