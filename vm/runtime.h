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

#include <lib/inst.h>

#include "./struct.h"

typedef enum
{
  ERR_OK = 0,
  ERR_STACK_UNDERFLOW,
  ERR_STACK_OVERFLOW,
  ERR_CALL_STACK_UNDERFLOW,
  ERR_CALL_STACK_OVERFLOW,
  ERR_INVALID_OPCODE,
  ERR_INVALID_REGISTER_BYTE,
  ERR_INVALID_REGISTER_HWORD,
  ERR_INVALID_REGISTER_WORD,
  ERR_INVALID_PROGRAM_ADDRESS,
  ERR_INVALID_PAGE_ADDRESS,
  ERR_OUT_OF_BOUNDS,
  ERR_END_OF_PROGRAM,
} err_t;

const char *err_as_cstr(err_t);

err_t vm_execute(vm_t *);
err_t vm_execute_all(vm_t *);

err_t vm_jump(vm_t *, word_t);

err_t vm_pop_byte(vm_t *, data_t *);
err_t vm_pop_hword(vm_t *, data_t *);
err_t vm_pop_word(vm_t *, data_t *);

typedef err_t (*pop_f)(vm_t *, data_t *);
static const pop_f POP_ROUTINES[] = {
    [OP_POP_BYTE]  = vm_pop_byte,
    [OP_POP_HWORD] = vm_pop_hword,
    [OP_POP_WORD]  = vm_pop_word,
};

err_t vm_push_byte(vm_t *, data_t);
err_t vm_push_hword(vm_t *, data_t);
err_t vm_push_word(vm_t *, data_t);

typedef err_t (*push_f)(vm_t *, data_t);
static const push_f PUSH_ROUTINES[] = {
    [OP_PUSH_BYTE]  = vm_push_byte,
    [OP_PUSH_HWORD] = vm_push_hword,
    [OP_PUSH_WORD]  = vm_push_word,
};

/* Operations where operand is a word from inst_t.operand  */
err_t vm_push_byte_register(vm_t *, word_t);
err_t vm_push_hword_register(vm_t *, word_t);
err_t vm_push_word_register(vm_t *, word_t);

err_t vm_mov_byte(vm_t *, word_t);
err_t vm_mov_hword(vm_t *, word_t);
err_t vm_mov_word(vm_t *, word_t);

err_t vm_dup_byte(vm_t *, word_t);
err_t vm_dup_hword(vm_t *, word_t);
err_t vm_dup_word(vm_t *, word_t);

err_t vm_malloc_byte(vm_t *, word_t);
err_t vm_malloc_hword(vm_t *, word_t);
err_t vm_malloc_word(vm_t *, word_t);

err_t vm_mset_byte(vm_t *, word_t);
err_t vm_mset_hword(vm_t *, word_t);
err_t vm_mset_word(vm_t *, word_t);

err_t vm_mget_byte(vm_t *, word_t);
err_t vm_mget_hword(vm_t *, word_t);
err_t vm_mget_word(vm_t *, word_t);

typedef err_t (*word_f)(vm_t *, word_t);
static const word_f WORD_ROUTINES[] = {
    [OP_PUSH_REGISTER_BYTE]  = vm_push_byte_register,
    [OP_PUSH_REGISTER_HWORD] = vm_push_hword_register,
    [OP_PUSH_REGISTER_WORD]  = vm_push_word_register,
    [OP_MOV_BYTE]            = vm_mov_byte,
    [OP_MOV_HWORD]           = vm_mov_hword,
    [OP_MOV_WORD]            = vm_mov_word,
    [OP_DUP_BYTE]            = vm_dup_byte,
    [OP_DUP_HWORD]           = vm_dup_hword,
    [OP_DUP_WORD]            = vm_dup_word,
    [OP_MALLOC_BYTE]         = vm_malloc_byte,
    [OP_MALLOC_HWORD]        = vm_malloc_hword,
    [OP_MALLOC_WORD]         = vm_malloc_word,
    [OP_MGET_BYTE]           = vm_mget_byte,
    [OP_MGET_HWORD]          = vm_mget_hword,
    [OP_MGET_WORD]           = vm_mget_word,
    [OP_MSET_BYTE]           = vm_mset_byte,
    [OP_MSET_HWORD]          = vm_mset_hword,
    [OP_MSET_WORD]           = vm_mset_word,
};

/* Operations that take input from the stack  */
err_t vm_malloc_stack_byte(vm_t *);
err_t vm_malloc_stack_hword(vm_t *);
err_t vm_malloc_stack_word(vm_t *);

err_t vm_mset_stack_byte(vm_t *);
err_t vm_mset_stack_hword(vm_t *);
err_t vm_mset_stack_word(vm_t *);

err_t vm_mget_stack_byte(vm_t *);
err_t vm_mget_stack_hword(vm_t *);
err_t vm_mget_stack_word(vm_t *);

err_t vm_mdelete(vm_t *);
err_t vm_msize(vm_t *);

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

err_t vm_sub_byte(vm_t *);
err_t vm_sub_hword(vm_t *);
err_t vm_sub_word(vm_t *);

err_t vm_mult_byte(vm_t *);
err_t vm_mult_hword(vm_t *);
err_t vm_mult_word(vm_t *);

typedef err_t (*stack_f)(vm_t *);
static const stack_f STACK_ROUTINES[] = {
    [OP_MALLOC_STACK_BYTE]  = vm_malloc_stack_byte,
    [OP_MALLOC_STACK_HWORD] = vm_malloc_stack_hword,
    [OP_MALLOC_STACK_WORD]  = vm_malloc_stack_word,
    [OP_MGET_STACK_BYTE]    = vm_mget_stack_byte,
    [OP_MGET_STACK_HWORD]   = vm_mget_stack_hword,
    [OP_MGET_STACK_WORD]    = vm_mget_stack_word,
    [OP_MSET_STACK_BYTE]    = vm_mset_stack_byte,
    [OP_MSET_STACK_HWORD]   = vm_mset_stack_hword,
    [OP_MSET_STACK_WORD]    = vm_mset_stack_word,
    [OP_MDELETE]            = vm_mdelete,
    [OP_MSIZE]              = vm_msize,

    [OP_NOT_BYTE]  = vm_not_byte,
    [OP_NOT_HWORD] = vm_not_hword,
    [OP_NOT_WORD]  = vm_not_word,

    [OP_OR_BYTE]  = vm_or_byte,
    [OP_OR_HWORD] = vm_or_hword,
    [OP_OR_WORD]  = vm_or_word,

    [OP_AND_BYTE]  = vm_and_byte,
    [OP_AND_HWORD] = vm_and_hword,
    [OP_AND_WORD]  = vm_and_word,

    [OP_XOR_BYTE]  = vm_xor_byte,
    [OP_XOR_HWORD] = vm_xor_hword,
    [OP_XOR_WORD]  = vm_xor_word,

    [OP_EQ_BYTE]  = vm_eq_byte,
    [OP_EQ_HWORD] = vm_eq_hword,
    [OP_EQ_WORD]  = vm_eq_word,

    [OP_LT_BYTE]   = vm_lt_byte,
    [OP_LT_SBYTE]  = vm_lt_sbyte,
    [OP_LT_SHWORD] = vm_lt_shword,
    [OP_LT_HWORD]  = vm_lt_hword,
    [OP_LT_SWORD]  = vm_lt_sword,
    [OP_LT_WORD]   = vm_lt_word,

    [OP_LTE_BYTE]   = vm_lte_byte,
    [OP_LTE_SBYTE]  = vm_lte_sbyte,
    [OP_LTE_SHWORD] = vm_lte_shword,
    [OP_LTE_HWORD]  = vm_lte_hword,
    [OP_LTE_SWORD]  = vm_lte_sword,
    [OP_LTE_WORD]   = vm_lte_word,

    [OP_GT_BYTE]   = vm_gt_byte,
    [OP_GT_SBYTE]  = vm_gt_sbyte,
    [OP_GT_SHWORD] = vm_gt_shword,
    [OP_GT_HWORD]  = vm_gt_hword,
    [OP_GT_SWORD]  = vm_gt_sword,
    [OP_GT_WORD]   = vm_gt_word,

    [OP_GTE_BYTE]   = vm_gte_byte,
    [OP_GTE_SBYTE]  = vm_gte_sbyte,
    [OP_GTE_SHWORD] = vm_gte_shword,
    [OP_GTE_HWORD]  = vm_gte_hword,
    [OP_GTE_SWORD]  = vm_gte_sword,
    [OP_GTE_WORD]   = vm_gte_word,

    [OP_PLUS_BYTE]  = vm_plus_byte,
    [OP_PLUS_HWORD] = vm_plus_hword,
    [OP_PLUS_WORD]  = vm_plus_word,
    [OP_SUB_BYTE]   = vm_sub_byte,
    [OP_SUB_HWORD]  = vm_sub_hword,
    [OP_SUB_WORD]   = vm_sub_word,

    [OP_MULT_BYTE]  = vm_mult_byte,
    [OP_MULT_HWORD] = vm_mult_hword,
    [OP_MULT_WORD]  = vm_mult_word,
};

#endif
