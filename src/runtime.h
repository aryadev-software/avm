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

#define VM_BYTE_REGISTERS  8
#define VM_WORD_REGISTERS  8
#define VM_FLOAT_REGISTERS 8
typedef struct
{
  struct Registers
  {
    word ret;
    byte b[VM_BYTE_REGISTERS];
    word w[VM_WORD_REGISTERS];
    f64 f[VM_FLOAT_REGISTERS];
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

void vm_execute(vm_t *);

void vm_load_stack(vm_t *, byte *, size_t);
void vm_load_program(vm_t *, inst_t *, size_t);

// Print routines
void vm_print_registers(vm_t *, FILE *);
void vm_print_stack(vm_t *, FILE *);
#define VM_PRINT_PROGRAM_EXCERPT 5
void vm_print_program(vm_t *, FILE *);
void vm_print_all(vm_t *, FILE *);

void vm_push_byte(vm_t *, data_t);
void vm_push_word(vm_t *, data_t);
void vm_push_float(vm_t *, data_t);

typedef void (*push_f)(vm_t *, data_t);
static const push_f PUSH_ROUTINES[] = {
    [OP_PUSH_BYTE]  = vm_push_byte,
    [OP_PUSH_WORD]  = vm_push_word,
    [OP_PUSH_FLOAT] = vm_push_float,
};

void vm_push_byte_register(vm_t *, word);
void vm_push_word_register(vm_t *, word);
void vm_push_float_register(vm_t *, word);

typedef void (*push_reg_f)(vm_t *, word);
static const push_reg_f PUSH_REG_ROUTINES[] = {
    [OP_PUSH_BYTE_REGISTER]  = vm_push_byte_register,
    [OP_PUSH_WORD_REGISTER]  = vm_push_word_register,
    [OP_PUSH_FLOAT_REGISTER] = vm_push_float_register,
};

data_t vm_mov_byte(vm_t *, word);
data_t vm_mov_word(vm_t *, word);
data_t vm_mov_float(vm_t *, word);

typedef data_t (*mov_f)(vm_t *, word);
static const mov_f MOV_ROUTINES[] = {
    [OP_MOV_BYTE]  = vm_mov_byte,
    [OP_MOV_WORD]  = vm_mov_word,
    [OP_MOV_FLOAT] = vm_mov_float,
};

data_t vm_pop_byte(vm_t *);
data_t vm_pop_word(vm_t *);
data_t vm_pop_float(vm_t *);

typedef data_t (*pop_f)(vm_t *);
static const pop_f POP_ROUTINES[] = {
    [OP_POP_BYTE]  = vm_pop_byte,
    [OP_POP_WORD]  = vm_pop_word,
    [OP_POP_FLOAT] = vm_pop_float,
};

#endif
