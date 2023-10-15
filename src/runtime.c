/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Virtual machine implementation
 */

#include <stdlib.h>
#include <string.h>

#include "./runtime.h"

void vm_execute(vm_t *vm)
{
  struct Program *prog = &vm->program;
  if (prog->ptr >= prog->max)
    // TODO: Error (Went past end of program)
    return;
  inst_t instruction = prog->instructions[prog->ptr];

  if (OPCODE_IS_TYPE(instruction.opcode, OP_TYPE_PUSH))
  {
    PUSH_ROUTINES[instruction.opcode](vm, instruction.operand);
    vm->registers.ret = instruction.operand.as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_TYPE_PUSH_REGISTER))
  {
    PUSH_REG_ROUTINES[instruction.opcode](vm, instruction.operand.as_word);
    vm->registers.ret = instruction.operand.as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_TYPE_POP))
  {
    // NOTE: We use the `ret` register for the result of this pop
    data_t d          = POP_ROUTINES[instruction.opcode](vm);
    vm->registers.ret = d.as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_TYPE_MOV))
  {
    data_t d =
        MOV_ROUTINES[instruction.opcode](vm, instruction.operand.as_word);
    vm->registers.ret = d.as_word; // will do type punning for me
    prog->ptr++;
  }
  else
  {
    // TODO: Error (Unknown opcode)
    return;
  }
}

void vm_load_stack(vm_t *vm, byte *bytes, size_t size)
{
  vm->stack.data = bytes;
  vm->stack.max  = size;
  vm->stack.ptr  = 0;
}

void vm_load_program(vm_t *vm, inst_t *instructions, size_t size)
{
  vm->program.instructions = instructions;
  vm->program.max          = size;
  vm->program.ptr          = 0;
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

void vm_push_byte_register(vm_t *vm, word reg)
{
  if (reg >= VM_BYTE_REGISTERS)
    // TODO: Error (reg is not a valid byte register)
    return;
  else if (vm->stack.ptr >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;
  vm_push_byte(vm, DBYTE(vm->registers.b[reg]));
}

void vm_push_word_register(vm_t *vm, word reg)
{
  if (reg >= VM_WORD_REGISTERS)
    // TODO: Error (reg is not a valid word register)
    return;
  else if (vm->stack.ptr >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;
  vm_push_word(vm, DWORD(vm->registers.w[reg]));
}

void vm_push_float_register(vm_t *vm, word reg)
{
  if (reg >= VM_FLOAT_REGISTERS)
    // TODO: Error (reg is not a valid float register)
    return;
  else if (vm->stack.ptr >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;
  vm_push_float(vm, DFLOAT(vm->registers.f[reg]));
}

data_t vm_mov_byte(vm_t *vm, word reg)
{
  if (reg >= VM_BYTE_REGISTERS)
    // TODO: Error (reg is not a valid byte register)
    return DBYTE(0);
  else if (vm->stack.ptr == 0)
    // TODO: Error (STACK UNDERFLOW)
    return DBYTE(0);
  data_t ret           = vm_pop_byte(vm);
  vm->registers.b[reg] = ret.as_byte;
  return ret;
}

data_t vm_mov_word(vm_t *vm, word reg)
{
  if (reg >= VM_WORD_REGISTERS)
    // TODO: Error (reg is not a valid word register)
    return DWORD(0);
  else if (vm->stack.ptr < sizeof(word))
    // TODO: Error (STACK UNDERFLOW)
    return DWORD(0);
  data_t ret           = vm_pop_word(vm);
  vm->registers.w[reg] = ret.as_word;
  return ret;
}

data_t vm_mov_float(vm_t *vm, word reg)
{
  if (reg >= VM_WORD_REGISTERS)
    // TODO: Error (reg is not a valid float register)
    return DFLOAT(0);
  else if (vm->stack.ptr < sizeof(f64))
    // TODO: Error (STACK UNDERFLOW)
    return DFLOAT(0);
  data_t ret           = vm_pop_float(vm);
  vm->registers.f[reg] = ret.as_float;
  return ret;
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
