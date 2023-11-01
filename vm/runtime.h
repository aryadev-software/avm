/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Virtual machine implementation
 */

#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdio.h>
#include <stdlib.h>

#include <lib/inst.h>

typedef enum
{
  ERR_OK = 0,
  ERR_STACK_UNDERFLOW,
  ERR_STACK_OVERFLOW,
  ERR_INVALID_OPCODE,
  ERR_INVALID_REGISTER_BYTE,
  ERR_INVALID_REGISTER_HWORD,
  ERR_INVALID_REGISTER_WORD,
  ERR_INVALID_PROGRAM_ADDRESS,
  ERR_END_OF_PROGRAM,
} err_t;

const char *err_as_cstr(err_t);

typedef darr_t registers_t;
#define VM_NTH_REGISTER(REGISTERS, N)     (((word *)((REGISTERS).data))[N])
#define VM_REGISTERS_AVAILABLE(REGISTERS) (((REGISTERS).available) / WORD_SIZE)

typedef struct
{
  registers_t registers;
  struct Stack
  {
    byte *data;
    word ptr, max;
  } stack;
  struct Program
  {
    inst_t *instructions;
    word ptr, max;
  } program;
} vm_t;

err_t vm_execute(vm_t *);
err_t vm_execute_all(vm_t *);

void vm_load_stack(vm_t *, byte *, size_t);
void vm_load_program(vm_t *, inst_t *, size_t);
void vm_load_registers(vm_t *, registers_t);

// Print routines
#define VM_PRINT_PROGRAM_EXCERPT 5
void vm_print_registers(vm_t *, FILE *);
void vm_print_stack(vm_t *, FILE *);
void vm_print_program(vm_t *, FILE *);
void vm_print_all(vm_t *, FILE *);

// Execution routines
err_t vm_jump(vm_t *, word);

err_t vm_pop_byte(vm_t *, data_t *);
err_t vm_pop_hword(vm_t *, data_t *);
err_t vm_pop_word(vm_t *, data_t *);

err_t vm_push_byte(vm_t *, data_t);
err_t vm_push_hword(vm_t *, data_t);
err_t vm_push_word(vm_t *, data_t);

typedef err_t (*push_f)(vm_t *, data_t);
static const push_f PUSH_ROUTINES[] = {
    [OP_PUSH_BYTE]  = vm_push_byte,
    [OP_PUSH_HWORD] = vm_push_hword,
    [OP_PUSH_WORD]  = vm_push_word,
};

err_t vm_push_byte_register(vm_t *, word);
err_t vm_push_hword_register(vm_t *, word);
err_t vm_push_word_register(vm_t *, word);

err_t vm_mov_byte(vm_t *, word);
err_t vm_mov_hword(vm_t *, word);
err_t vm_mov_word(vm_t *, word);

typedef err_t (*reg_f)(vm_t *, word);
static const reg_f REG_ROUTINES[] = {
    [OP_PUSH_REGISTER_BYTE]  = vm_push_byte_register,
    [OP_PUSH_REGISTER_HWORD] = vm_push_hword_register,
    [OP_PUSH_REGISTER_WORD]  = vm_push_word_register,
    [OP_MOV_BYTE]            = vm_mov_byte,
    [OP_MOV_HWORD]           = vm_mov_hword,
    [OP_MOV_WORD]            = vm_mov_word,
};

err_t vm_dup_byte(vm_t *, word);
err_t vm_dup_hword(vm_t *, word);
err_t vm_dup_word(vm_t *, word);

typedef err_t (*dup_f)(vm_t *, word);
static const dup_f DUP_ROUTINES[] = {
    [OP_DUP_BYTE]  = vm_dup_byte,
    [OP_DUP_HWORD] = vm_dup_hword,
    [OP_DUP_WORD]  = vm_dup_word,
};

err_t vm_not_byte(vm_t *);
err_t vm_not_hword(vm_t *);
err_t vm_not_word(vm_t *);

err_t vm_or_byte(vm_t *);
err_t vm_or_hword(vm_t *);
err_t vm_or_word(vm_t *);

err_t vm_and_byte(vm_t *);
err_t vm_and_hword(vm_t *);
err_t vm_and_word(vm_t *);

err_t vm_xor_byte(vm_t *);
err_t vm_xor_hword(vm_t *);
err_t vm_xor_word(vm_t *);

err_t vm_eq_byte(vm_t *);
err_t vm_eq_char(vm_t *);
err_t vm_eq_int(vm_t *);
err_t vm_eq_hword(vm_t *);
err_t vm_eq_long(vm_t *);
err_t vm_eq_word(vm_t *);

err_t vm_lt_byte(vm_t *);
err_t vm_lt_char(vm_t *);
err_t vm_lt_int(vm_t *);
err_t vm_lt_hword(vm_t *);
err_t vm_lt_long(vm_t *);
err_t vm_lt_word(vm_t *);

err_t vm_lte_byte(vm_t *);
err_t vm_lte_char(vm_t *);
err_t vm_lte_int(vm_t *);
err_t vm_lte_hword(vm_t *);
err_t vm_lte_long(vm_t *);
err_t vm_lte_word(vm_t *);

err_t vm_gt_byte(vm_t *);
err_t vm_gt_char(vm_t *);
err_t vm_gt_int(vm_t *);
err_t vm_gt_hword(vm_t *);
err_t vm_gt_long(vm_t *);
err_t vm_gt_word(vm_t *);

err_t vm_gte_byte(vm_t *);
err_t vm_gte_char(vm_t *);
err_t vm_gte_int(vm_t *);
err_t vm_gte_hword(vm_t *);
err_t vm_gte_long(vm_t *);
err_t vm_gte_word(vm_t *);

err_t vm_plus_byte(vm_t *);
err_t vm_plus_hword(vm_t *);
err_t vm_plus_word(vm_t *);

typedef err_t (*stack_f)(vm_t *);
static const stack_f STACK_ROUTINES[] = {
    [OP_NOT_BYTE] = vm_not_byte,   [OP_NOT_HWORD] = vm_not_hword,
    [OP_NOT_WORD] = vm_not_word,

    [OP_OR_BYTE] = vm_or_byte,     [OP_OR_HWORD] = vm_or_hword,
    [OP_OR_WORD] = vm_or_word,

    [OP_AND_BYTE] = vm_and_byte,   [OP_AND_HWORD] = vm_and_hword,
    [OP_AND_WORD] = vm_and_word,

    [OP_XOR_BYTE] = vm_xor_byte,   [OP_XOR_HWORD] = vm_xor_hword,
    [OP_XOR_WORD] = vm_xor_word,

    [OP_EQ_BYTE] = vm_eq_byte,     [OP_EQ_HWORD] = vm_eq_hword,
    [OP_EQ_WORD] = vm_eq_word,

    [OP_LT_BYTE] = vm_lt_byte,     [OP_LT_CHAR] = vm_lt_char,
    [OP_LT_INT] = vm_lt_int,       [OP_LT_HWORD] = vm_lt_hword,
    [OP_LT_LONG] = vm_lt_long,     [OP_LT_WORD] = vm_lt_word,

    [OP_LTE_BYTE] = vm_lte_byte,   [OP_LTE_CHAR] = vm_lte_char,
    [OP_LTE_INT] = vm_lte_int,     [OP_LTE_HWORD] = vm_lte_hword,
    [OP_LTE_LONG] = vm_lte_long,   [OP_LTE_WORD] = vm_lte_word,

    [OP_GT_BYTE] = vm_gt_byte,     [OP_GT_CHAR] = vm_gt_char,
    [OP_GT_INT] = vm_gt_int,       [OP_GT_HWORD] = vm_gt_hword,
    [OP_GT_LONG] = vm_gt_long,     [OP_GT_WORD] = vm_gt_word,

    [OP_GTE_BYTE] = vm_gte_byte,   [OP_GTE_CHAR] = vm_gte_char,
    [OP_GTE_INT] = vm_gte_int,     [OP_GTE_HWORD] = vm_gte_hword,
    [OP_GTE_LONG] = vm_gte_long,   [OP_GTE_WORD] = vm_gte_word,

    [OP_PLUS_BYTE] = vm_plus_byte, [OP_PLUS_HWORD] = vm_plus_hword,
    [OP_PLUS_WORD] = vm_plus_word,
};

#endif
