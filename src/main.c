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

typedef struct
{
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
    printf("PUSH(%lu): pushed byte %X\n", i, b);
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

byte vm_pop_byte(vm_t *vm)
{
  if (vm->stack.ptr == 0)
    // TODO: Error STACK_UNDERFLOW
    return 0;
  return vm->stack.data[--vm->stack.ptr];
}

word vm_pop_word(vm_t *vm)
{
  if (vm->stack.ptr < WORD_SIZE)
    // TODO: Error STACK_UNDERFLOW
    return 0;
  word w = 0;
  for (size_t i = 0; i < WORD_SIZE; ++i)
  {
    byte b = vm_pop_byte(vm);
    printf("POP(%lu): popped byte %X\n", i, b);
    w = w | ((word)b << (i * 8));
  }
  return w;
}

f64 vm_pop_float(vm_t *vm)
{
  if (vm->stack.ptr < FLOAT_SIZE)
    // TODO: Error STACK_UNDERFLOW
    return 0;
  f64 f = 0;
  // TODO: Make this machine independent (encode IEEE754 floats
  // yourself?)
  memcpy(&f, vm->stack.data + vm->stack.ptr - FLOAT_SIZE, FLOAT_SIZE);
  vm->stack.ptr -= FLOAT_SIZE;
  return f;
}

typedef void (*push_f)(vm_t *, data_t);

void vm_execute(vm_t *vm)
{
  struct Program *prog = &vm->program;
  if (prog->ptr >= prog->max)
    // TODO: Error (Went past end of program)
    return;
  inst_t instruction = prog->instructions[prog->ptr];
  // NOTE: This is ballsy; I'm essentially saying I will never use the
  // last 2 bits unless it's a push routine
  if ((instruction.opcode & 0b11) != 0)
  {
    // Push routine
    const push_f routines[] = {[OP_PUSH_BYTE]  = vm_push_byte,
                               [OP_PUSH_WORD]  = vm_push_word,
                               [OP_PUSH_FLOAT] = vm_push_float};
    routines[instruction.opcode](vm, instruction.operand);
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
