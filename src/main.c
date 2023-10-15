/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Entrypoint to program
 */

#include <stdio.h>
#include <string.h>

#include "./base.h"
#include "./inst.h"

#define VM_BYTE_REGISTERS  8
#define VM_WORD_REGISTERS  8
#define VM_FLOAT_REGISTERS 8
typedef struct
{
  struct Registers
  {
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

void vm_load_program(vm_t *vm, inst_t *instructions, size_t size)
{
  vm->program.instructions = instructions;
  vm->program.max          = size;
  vm->program.ptr          = 0;
}

void vm_load_stack(vm_t *vm, byte *bytes, size_t size)
{
  vm->stack.data = bytes;
  vm->stack.max  = size;
  vm->stack.ptr  = 0;
}

void vm_push_byte(vm_t *vm, data_t b)
{
  if (vm->stack.ptr >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;
  vm->stack.data[vm->stack.ptr++] = b.as_byte;
}

void vm_push_word(vm_t *vm, data_t w)
{
  if (vm->stack.ptr + WORD_SIZE >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;
  // By default store in big endian
  for (size_t i = 64; i > 0; i -= 8)
  {
    const word mask = ((word)0b11111111) << (i - 8);
    byte b          = (w.as_word & mask) >> (i - 8);
    vm_push_byte(vm, DBYTE(b));
  }
}

void vm_push_float(vm_t *vm, data_t f)
{
  if (vm->stack.ptr + FLOAT_SIZE >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;
  // TODO: Make this machine independent (encode IEEE754 floats
  // yourself?)
  memcpy(vm->stack.data + vm->stack.ptr, &f.as_float, FLOAT_SIZE);
  vm->stack.ptr += FLOAT_SIZE;
}

void vm_mov_byte(vm_t *vm, data_t b, word reg)
{
  if (reg >= VM_BYTE_REGISTERS)
    // TODO: Error (reg is not a valid byte register)
    return;
  vm->registers.b[reg] = b.as_byte;
}

void vm_mov_word(vm_t *vm, data_t w, word reg)
{
  if (reg >= VM_WORD_REGISTERS)
    // TODO: Error (reg is not a valid byte register)
    return;
  vm->registers.w[reg] = w.as_word;
}

void vm_mov_float(vm_t *vm, data_t f, word reg)
{
  if (reg >= VM_FLOAT_REGISTERS)
    // TODO: Error (reg is not a valid byte register)
    return;
  vm->registers.f[reg] = f.as_float;
}

data_t vm_pop_byte(vm_t *vm)
{
  if (vm->stack.ptr == 0)
    // TODO: Error STACK_UNDERFLOW
    return DBYTE(0);
  return DBYTE(vm->stack.data[--vm->stack.ptr]);
}

data_t vm_pop_word(vm_t *vm)
{
  if (vm->stack.ptr < WORD_SIZE)
    // TODO: Error STACK_UNDERFLOW
    return DWORD(0);
  word w = 0;
  for (size_t i = 0; i < WORD_SIZE; ++i)
  {
    data_t b = vm_pop_byte(vm);
    w        = w | ((word)(b.as_byte) << (i * 8));
  }
  return DWORD(w);
}

data_t vm_pop_float(vm_t *vm)
{
  if (vm->stack.ptr < FLOAT_SIZE)
    // TODO: Error STACK_UNDERFLOW
    return DFLOAT(0);
  f64 f = 0;
  // TODO: Make this machine independent (encode IEEE754 floats
  // yourself?)
  memcpy(&f, vm->stack.data + vm->stack.ptr - FLOAT_SIZE, FLOAT_SIZE);
  vm->stack.ptr -= FLOAT_SIZE;
  return DFLOAT(f);
}

typedef void (*push_f)(vm_t *, data_t);
typedef void (*mov_f)(vm_t *, data_t, word);

static const push_f PUSH_ROUTINES[] = {
    [OP_PUSH_BYTE]  = vm_push_byte,
    [OP_PUSH_WORD]  = vm_push_word,
    [OP_PUSH_FLOAT] = vm_push_float,
};

static const mov_f MOV_ROUTINES[] = {
    [OP_MOV_BYTE]  = vm_mov_byte,
    [OP_MOV_WORD]  = vm_mov_word,
    [OP_MOV_FLOAT] = vm_mov_float,
};

void vm_execute(vm_t *vm)
{
  struct Program *prog = &vm->program;
  if (prog->ptr >= prog->max)
    // TODO: Error (Went past end of program)
    return;
  inst_t instruction = prog->instructions[prog->ptr];

  if (OPCODE_IS_PUSH(instruction.opcode))
  {
    PUSH_ROUTINES[instruction.opcode](vm, instruction.operand);
    prog->ptr++;
  }
  else if (OPCODE_IS_MOV(instruction.opcode))
  {
    MOV_ROUTINES[instruction.opcode](vm, instruction.operand, instruction.reg);
    prog->ptr++;
  }
  else
  {
    // TODO: Error (Unknown opcode)
    return;
  }
}

#define ARR_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))

int main(void)
{
  byte stack_data[256];
  vm_t vm = {0};
  vm_load_stack(&vm, stack_data, ARR_SIZE(stack_data));
  return 0;
}
