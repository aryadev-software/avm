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

#include "./base.h"
#include "./inst.h"

#define VM_REGISTERS 8
typedef struct
{
  struct Registers
  {
    // Used for internal
    word ret;
    // General registers
    word reg[VM_REGISTERS];
  } registers;
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

#define VM_REG_BYTE(REG)  ((REG)&0b11111111)
#define VM_REG_HWORD(REG) ((REG)&0b11111111111111111111111111111111)
#define VM_REG_WORD(REG)  ((REG))

void vm_execute(vm_t *);
void vm_execute_all(vm_t *);

void vm_load_stack(vm_t *, byte *, size_t);
void vm_load_program(vm_t *, inst_t *, size_t);

// Print routines
#define VM_PRINT_PROGRAM_EXCERPT 5
void vm_print_registers(vm_t *, FILE *);
void vm_print_stack(vm_t *, FILE *);
void vm_print_program(vm_t *, FILE *);
void vm_print_all(vm_t *, FILE *);

data_t vm_peek(vm_t *, data_type_t);

// Execution routines
void vm_push_byte(vm_t *, data_t);
void vm_push_hword(vm_t *, data_t);
void vm_push_word(vm_t *, data_t);

typedef void (*push_f)(vm_t *, data_t);
static const push_f PUSH_ROUTINES[] = {
    [OP_PUSH_BYTE]  = vm_push_byte,
    [OP_PUSH_HWORD] = vm_push_hword,
    [OP_PUSH_WORD]  = vm_push_word,
};

void vm_push_byte_register(vm_t *, byte);
void vm_push_hword_register(vm_t *, byte);
void vm_push_word_register(vm_t *, byte);

typedef void (*push_reg_f)(vm_t *, byte);
static const push_reg_f PUSH_REG_ROUTINES[] = {
    [OP_PUSH_REGISTER_BYTE]  = vm_push_byte_register,
    [OP_PUSH_REGISTER_HWORD] = vm_push_hword_register,
    [OP_PUSH_REGISTER_WORD]  = vm_push_word_register,
};

data_t vm_pop_byte(vm_t *);
data_t vm_pop_hword(vm_t *);
data_t vm_pop_word(vm_t *);

data_t vm_mov_byte(vm_t *, byte);
data_t vm_mov_hword(vm_t *, byte);
data_t vm_mov_word(vm_t *, byte);

typedef data_t (*mov_f)(vm_t *, byte);
static const mov_f MOV_ROUTINES[] = {
    [OP_MOV_BYTE]  = vm_mov_byte,
    [OP_MOV_HWORD] = vm_mov_hword,
    [OP_MOV_WORD]  = vm_mov_word,
};

void vm_dup_byte(vm_t *, word);
void vm_dup_hword(vm_t *, word);
void vm_dup_word(vm_t *, word);

typedef void (*dup_f)(vm_t *, word);
static const dup_f DUP_ROUTINES[] = {
    [OP_DUP_BYTE]  = vm_dup_byte,
    [OP_DUP_HWORD] = vm_dup_hword,
    [OP_DUP_WORD]  = vm_dup_word,
};

void vm_not_byte(vm_t *);
void vm_not_hword(vm_t *);
void vm_not_word(vm_t *);

typedef void (*not_f)(vm_t *);
static const not_f NOT_ROUTINES[] = {
    [OP_NOT_BYTE]  = vm_not_byte,
    [OP_NOT_HWORD] = vm_not_hword,
    [OP_NOT_WORD]  = vm_not_word,
};

void vm_or_byte(vm_t *);
void vm_or_hword(vm_t *);
void vm_or_word(vm_t *);
typedef void (*or_f)(vm_t *);
static const or_f OR_ROUTINES[] = {
    [OP_OR_BYTE]  = vm_or_byte,
    [OP_OR_HWORD] = vm_or_hword,
    [OP_OR_WORD]  = vm_or_word,
};

void vm_and_byte(vm_t *);
void vm_and_hword(vm_t *);
void vm_and_word(vm_t *);
typedef void (*and_f)(vm_t *);
static const and_f AND_ROUTINES[] = {
    [OP_AND_BYTE]  = vm_and_byte,
    [OP_AND_HWORD] = vm_and_hword,
    [OP_AND_WORD]  = vm_and_word,
};

void vm_xor_byte(vm_t *);
void vm_xor_hword(vm_t *);
void vm_xor_word(vm_t *);
typedef void (*xor_f)(vm_t *);
static const xor_f XOR_ROUTINES[] = {
    [OP_XOR_BYTE]  = vm_xor_byte,
    [OP_XOR_HWORD] = vm_xor_hword,
    [OP_XOR_WORD]  = vm_xor_word,
};

void vm_eq_byte(vm_t *);
void vm_eq_hword(vm_t *);
void vm_eq_word(vm_t *);
typedef void (*eq_f)(vm_t *);
static const eq_f EQ_ROUTINES[] = {
    [OP_EQ_BYTE]  = vm_eq_byte,
    [OP_EQ_HWORD] = vm_eq_hword,
    [OP_EQ_WORD]  = vm_eq_word,
};

#endif
