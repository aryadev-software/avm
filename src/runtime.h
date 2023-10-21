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
    word ret;
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
void vm_print_registers(vm_t *, FILE *);
void vm_print_stack(vm_t *, FILE *);
#define VM_PRINT_PROGRAM_EXCERPT 5
void vm_print_program(vm_t *, FILE *);
void vm_print_all(vm_t *, FILE *);

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

data_t vm_mov_byte(vm_t *, byte);
data_t vm_mov_hword(vm_t *, byte);
data_t vm_mov_word(vm_t *, byte);

typedef data_t (*mov_f)(vm_t *, byte);
static const mov_f MOV_ROUTINES[] = {
    [OP_MOV_BYTE]  = vm_mov_byte,
    [OP_MOV_HWORD] = vm_mov_hword,
    [OP_MOV_WORD]  = vm_mov_word,
};

data_t vm_pop_byte(vm_t *);
data_t vm_pop_hword(vm_t *);
data_t vm_pop_word(vm_t *);

typedef data_t (*pop_f)(vm_t *);
static const pop_f POP_ROUTINES[] = {
    [OP_POP_BYTE]  = vm_pop_byte,
    [OP_POP_HWORD] = vm_pop_hword,
    [OP_POP_WORD]  = vm_pop_word,
};

#endif
