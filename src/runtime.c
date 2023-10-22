/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Virtual machine implementation
 */

#include <stdio.h>
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

  if (OPCODE_IS_TYPE(instruction.opcode, OP_PUSH))
  {
    PUSH_ROUTINES[instruction.opcode](vm, instruction.operand);
    vm->registers.ret = instruction.operand.as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_PUSH_REGISTER))
  {
    PUSH_REG_ROUTINES[instruction.opcode](vm, instruction.operand.as_word);
    vm->registers.ret = instruction.operand.as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_POP))
  {
    // NOTE: We use the first register to hold the result of this pop
    data_type_t type = OPCODE_DATA_TYPE(instruction.opcode, OP_POP);
    data_t datum     = vm_peek(vm, type);
    switch (type)
    {
    case DATA_TYPE_NIL:
      break;
    case DATA_TYPE_BYTE:
      vm_mov_byte(vm, (VM_REGISTERS * 8) - 1);
      break;
    case DATA_TYPE_HWORD:
      vm_mov_hword(vm, (VM_REGISTERS * 4) - 1);
      break;
    case DATA_TYPE_WORD:
      vm_mov_hword(vm, VM_REGISTERS - 1);
      break;
    }
    vm->registers.ret = datum.as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_MOV))
  {
    data_t d =
        MOV_ROUTINES[instruction.opcode](vm, instruction.operand.as_word);
    vm->registers.ret = d.as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_NOT))
  {
    NOT_ROUTINES[instruction.opcode](vm);
    vm->registers.ret =
        vm_peek(vm, OPCODE_DATA_TYPE(instruction.opcode, OP_NOT)).as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_OR))
  {
    OR_ROUTINES[instruction.opcode](vm);
    vm->registers.ret =
        vm_peek(vm, OPCODE_DATA_TYPE(instruction.opcode, OP_OR)).as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_AND))
  {
    AND_ROUTINES[instruction.opcode](vm);
    vm->registers.ret =
        vm_peek(vm, OPCODE_DATA_TYPE(instruction.opcode, OP_AND)).as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_XOR))
  {
    XOR_ROUTINES[instruction.opcode](vm);
    vm->registers.ret =
        vm_peek(vm, OPCODE_DATA_TYPE(instruction.opcode, OP_XOR)).as_word;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_EQ))
  {
    EQ_ROUTINES[instruction.opcode](vm);
    vm->registers.ret =
        vm_peek(vm, OPCODE_DATA_TYPE(instruction.opcode, OP_EQ)).as_word;
    prog->ptr++;
  }
  else if (instruction.opcode == OP_HALT)
  {
    // Do nothing here.  Should be caught by callers of vm_execute
  }
  else
  {
    // TODO: Error (Unknown opcode)
    return;
  }
}

void vm_execute_all(vm_t *vm)
{
  struct Program *program = &vm->program;
#if VERBOSE == 1
  struct Registers prev_registers = vm->registers;
  size_t cycles                   = 0;
  size_t prev_sptr                = 0;
#endif
  while (program->instructions[program->ptr].opcode != OP_HALT)
  {
#if VERBOSE >= 1
    fprintf(stdout, "[vm_execute_all]: Trace(Cycle %lu)\n", cycles);
    fputs(
        "----------------------------------------------------------------------"
        "----------\n",
        stdout);
    vm_print_program(vm, stdout);
    fputs(
        "----------------------------------------------------------------------"
        "----------\n",
        stdout);
    if (memcmp(prev_registers.reg, vm->registers.reg,
               ARR_SIZE(vm->registers.reg)) != 0)
    {
      vm_print_registers(vm, stdout);
      prev_registers = vm->registers;
      fputs("------------------------------------------------------------------"
            "----"
            "----------\n",
            stdout);
    }
    if (prev_sptr != vm->stack.ptr)
    {
      vm_print_stack(vm, stdout);
      prev_sptr = vm->stack.ptr;
      fputs("------------------------------------------------------------------"
            "----"
            "----------\n",
            stdout);
    }
    ++cycles;
#endif
    vm_execute(vm);
  }

#if VERBOSE >= 1
  fprintf(stdout, "[vm_execute_all]: Final VM state(Cycle %lu)\n", cycles);
  vm_print_all(vm, stdout);
#endif
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

void vm_print_registers(vm_t *vm, FILE *fp)
{
  struct Registers reg = vm->registers;
  fprintf(fp, "Registers.ret = 0x%lX\n", reg.ret);
  fprintf(fp, "Registers.reg = [");
  for (size_t i = 0; i < VM_REGISTERS; ++i)
  {
    fprintf(fp, "{%lu:%lX}", i, reg.reg[i]);
    if (i != VM_REGISTERS - 1)
      fprintf(fp, ", ");
  }
  fprintf(fp, "]\n");
}

void vm_print_stack(vm_t *vm, FILE *fp)
{
  struct Stack stack = vm->stack;
  fprintf(fp, "Stack.max  = %lu\nStack.ptr  = %lu\nStack.data = [", stack.max,
          stack.ptr);
  if (stack.ptr == 0)
  {
    fprintf(fp, "]\n");
    return;
  }
  printf("\n");
  for (size_t i = stack.ptr; i > 0; --i)
  {
    byte b = stack.data[i - 1];
    fprintf(fp, "\t%lu: %X", stack.ptr - i, b);
    if (i != 1)
      fprintf(fp, ", ");
    fprintf(fp, "\n");
  }
  fprintf(fp, "]\n");
}

void vm_print_program(vm_t *vm, FILE *fp)
{
  struct Program program = vm->program;
  fprintf(fp,
          "Program.max          = %lu\nProgram.ptr          = "
          "%lu\nProgram.instructions = [\n",
          program.max, program.ptr);
  size_t beg = 0;
  if (program.ptr >= VM_PRINT_PROGRAM_EXCERPT)
  {
    fprintf(fp, "\t...\n");
    beg = program.ptr - VM_PRINT_PROGRAM_EXCERPT;
  }
  else
    beg = 0;
  size_t end = MIN(program.ptr + VM_PRINT_PROGRAM_EXCERPT, program.max);
  for (size_t i = beg; i < end; ++i)
  {
    fprintf(fp, "\t%lu: ", i);
    inst_print(program.instructions[i], stdout);
    if (i == program.ptr)
      fprintf(fp, " <---");
    fprintf(fp, "\n");
  }
  if (end != program.max)
    fprintf(fp, "\t...\n");
  fprintf(fp, "]\n");
}

void vm_print_all(vm_t *vm, FILE *fp)
{
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
  vm_print_registers(vm, fp);
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
  vm_print_stack(vm, fp);
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
  vm_print_program(vm, fp);
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
}

data_t vm_peek(vm_t *vm, data_type_t type)
{
  switch (type)
  {
  case DATA_TYPE_NIL:
    return DBYTE(0);
    break;
  case DATA_TYPE_BYTE:
    if (vm->stack.ptr == 0)
      return DBYTE(0);
    return DBYTE(vm->stack.data[vm->stack.ptr - 1]);
    break;
  case DATA_TYPE_HWORD:
    if (vm->stack.ptr < HWORD_SIZE)
      // TODO: Error STACK_UNDERFLOW
      return DHWORD(0);
    hword h = 0;
    for (size_t i = 0; i < HWORD_SIZE; ++i)
    {
      byte b = vm->stack.data[vm->stack.ptr - 1 - i];
      h      = h | (((word)b) << (i * 8));
    }
    return DWORD(h);
    break;
  case DATA_TYPE_WORD:
    if (vm->stack.ptr < WORD_SIZE)
      // TODO: Error STACK_UNDERFLOW
      return DWORD(0);
    word w = 0;
    for (size_t i = 0; i < WORD_SIZE; ++i)
    {
      byte b = vm->stack.data[vm->stack.ptr - 1 - i];
      w      = w | (((word)b) << (i * 8));
    }
    return DWORD(w);
    break;
  default:
    return DBYTE(0);
    break;
  }
}

void vm_push_byte(vm_t *vm, data_t b)
{
  if (vm->stack.ptr >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;
  vm->stack.data[vm->stack.ptr++] = b.as_byte;
}

void vm_push_hword(vm_t *vm, data_t f)
{
  if (vm->stack.ptr + HWORD_SIZE >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;
  for (size_t i = 32; i > 0; i -= 8)
  {
    const word mask = ((word)0b11111111) << (i - 8);
    byte b          = (f.as_hword & mask) >> (i - 8);
    vm_push_byte(vm, DBYTE(b));
  }
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

#define WORD_NTH_BYTE(WORD, N) (((WORD) >> ((N)*8)) & 0b11111111)
#define WORD_NTH_HWORD(WORD, N) \
  (((WORD) >> ((N)*2)) & 0b11111111111111111111111111111111)

void vm_push_byte_register(vm_t *vm, byte reg)
{
  if (reg >= VM_REGISTERS * 8)
    // TODO: Error (reg is not a valid byte register)
    return;
  else if (vm->stack.ptr >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;

  // Interpret each word based register as 8 byte registers
  byte b = WORD_NTH_BYTE(vm->registers.reg[reg / 8], reg % 8);

  vm_push_byte(vm, DBYTE(b));
}

void vm_push_hword_register(vm_t *vm, byte reg)
{
  if (reg >= VM_REGISTERS * 2)
    // TODO: Error (reg is not a valid hword register)
    return;
  else if (vm->stack.ptr >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;
  // Interpret each word based register as 2 hword registers
  hword hw = WORD_NTH_HWORD(vm->registers.reg[reg / 2], reg % 2);
  vm_push_hword(vm, DHWORD(hw));
}

void vm_push_word_register(vm_t *vm, byte reg)
{
  if (reg >= VM_REGISTERS)
    // TODO: Error (reg is not a valid word register)
    return;
  else if (vm->stack.ptr >= vm->stack.max)
    // TODO: Error STACK_OVERFLOW
    return;
  vm_push_word(vm, DWORD(vm->registers.reg[reg]));
}

data_t vm_mov_byte(vm_t *vm, byte reg)
{
  if (reg >= VM_REGISTERS)
    // TODO: Error (reg is not a valid byte register)
    return DBYTE(0);
  else if (vm->stack.ptr == 0)
    // TODO: Error (STACK UNDERFLOW)
    return DBYTE(0);
  data_t ret    = vm_pop_byte(vm);
  word *reg_ptr = &vm->registers.reg[reg / 8];
  *reg_ptr      = (*reg_ptr) | (ret.as_word << ((reg % 8) * 8));
  return ret;
}

data_t vm_mov_hword(vm_t *vm, byte reg)
{
  if (reg >= VM_REGISTERS)
    // TODO: Error (reg is not a valid hword register)
    return DHWORD(0);
  else if (vm->stack.ptr < sizeof(f64))
    // TODO: Error (STACK UNDERFLOW)
    return DHWORD(0);
  data_t ret    = vm_pop_hword(vm);
  word *reg_ptr = &vm->registers.reg[reg / 2];
  *reg_ptr      = (*reg_ptr) | (ret.as_word << ((reg % 2) * 2));
  return ret;
}

data_t vm_mov_word(vm_t *vm, byte reg)
{
  if (reg >= VM_REGISTERS)
    // TODO: Error (reg is not a valid word register)
    return DWORD(0);
  else if (vm->stack.ptr < sizeof(word))
    // TODO: Error (STACK UNDERFLOW)
    return DWORD(0);
  data_t ret             = vm_pop_word(vm);
  vm->registers.reg[reg] = ret.as_word;
  return ret;
}

data_t vm_pop_byte(vm_t *vm)
{
  if (vm->stack.ptr == 0)
    // TODO: Error STACK_UNDERFLOW
    return DBYTE(0);
  return DBYTE(vm->stack.data[--vm->stack.ptr]);
}

data_t vm_pop_hword(vm_t *vm)
{
  if (vm->stack.ptr < HWORD_SIZE)
    // TODO: Error STACK_UNDERFLOW
    return DHWORD(0);
  hword h = 0;
  for (size_t i = 0; i < HWORD_SIZE; ++i)
  {
    data_t b = vm_pop_byte(vm);
    h        = h | ((word)(b.as_byte) << (i * 8));
  }
  return DWORD(h);
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

void vm_not_byte(vm_t *vm)
{
  if (vm->stack.ptr == 0)
    // TODO: Error STACK_UNDERFLOW
    return;

  byte a = vm_pop_byte(vm).as_byte;
  vm_push_byte(vm, DBYTE(!a));
}

void vm_not_hword(vm_t *vm)
{
  if (vm->stack.ptr < HWORD_SIZE)
    // TODO: Error STACK_UNDERFLOW
    return;

  hword a = vm_pop_hword(vm).as_hword;
  vm_push_hword(vm, DHWORD(!a));
}

void vm_not_word(vm_t *vm)
{
  if (vm->stack.ptr < WORD_SIZE)
    // TODO: Error STACK_UNDERFLOW
    return;

  word a = vm_pop_word(vm).as_word;
  vm_push_word(vm, DWORD(!a));
}

void vm_or_byte(vm_t *vm)
{
  if (vm->stack.ptr < 2)
    // TODO: Error STACK_UNDERFLOW
    return;
  byte a = vm_pop_byte(vm).as_byte;
  byte b = vm_pop_byte(vm).as_byte;
  vm_push_byte(vm, DBYTE(a | b));
}

void vm_or_hword(vm_t *vm)
{
  if (vm->stack.ptr < (HWORD_SIZE * 2))
    // TODO: Error STACK_UNDERFLOW
    return;
  hword a = vm_pop_hword(vm).as_hword;
  hword b = vm_pop_hword(vm).as_hword;
  vm_push_hword(vm, DHWORD(a | b));
}

void vm_or_word(vm_t *vm)
{
  if (vm->stack.ptr < (WORD_SIZE * 2))
    // TODO: Error STACK_UNDERFLOW
    return;
  word a = vm_pop_word(vm).as_word;
  word b = vm_pop_word(vm).as_word;
  vm_push_word(vm, DWORD(a | b));
}

void vm_and_byte(vm_t *vm)
{
  if (vm->stack.ptr < 2)
    // TODO: Error STACK_UNDERFLOW
    return;
  byte a = vm_pop_byte(vm).as_byte;
  byte b = vm_pop_byte(vm).as_byte;
  vm_push_byte(vm, DBYTE(a & b));
}

void vm_and_hword(vm_t *vm)
{
  if (vm->stack.ptr < (HWORD_SIZE * 2))
    // TODO: Error STACK_UNDERFLOW
    return;
  hword a = vm_pop_hword(vm).as_hword;
  hword b = vm_pop_hword(vm).as_hword;
  vm_push_hword(vm, DHWORD(a & b));
}

void vm_and_word(vm_t *vm)
{
  if (vm->stack.ptr < (WORD_SIZE * 2))
    // TODO: Error STACK_UNDERFLOW
    return;
  word a = vm_pop_word(vm).as_word;
  word b = vm_pop_word(vm).as_word;
  vm_push_word(vm, DWORD(a & b));
}

void vm_xor_byte(vm_t *vm)
{
  if (vm->stack.ptr < 2)
    // TODO: Error STACK_UNDERFLOW
    return;
  byte a = vm_pop_byte(vm).as_byte;
  byte b = vm_pop_byte(vm).as_byte;
  vm_push_byte(vm, DBYTE(a ^ b));
}

void vm_xor_hword(vm_t *vm)
{
  if (vm->stack.ptr < (HWORD_SIZE * 2))
    // TODO: Error STACK_UNDERFLOW
    return;
  hword a = vm_pop_hword(vm).as_hword;
  hword b = vm_pop_hword(vm).as_hword;
  vm_push_hword(vm, DHWORD(a ^ b));
}

void vm_xor_word(vm_t *vm)
{
  if (vm->stack.ptr < (WORD_SIZE * 2))
    // TODO: Error STACK_UNDERFLOW
    return;
  word a = vm_pop_word(vm).as_word;
  word b = vm_pop_word(vm).as_word;
  vm_push_word(vm, DWORD(a ^ b));
}

void vm_eq_byte(vm_t *vm)
{
  if (vm->stack.ptr < 2)
    // TODO: Error STACK_UNDERFLOW
    return;
  byte a = vm_pop_byte(vm).as_byte;
  byte b = vm_pop_byte(vm).as_byte;
  vm_push_byte(vm, DBYTE(a == b));
}

void vm_eq_hword(vm_t *vm)
{
  if (vm->stack.ptr < (HWORD_SIZE * 2))
    // TODO: Error STACK_UNDERFLOW
    return;
  hword a = vm_pop_hword(vm).as_hword;
  hword b = vm_pop_hword(vm).as_hword;
  vm_push_hword(vm, DHWORD(a == b));
}

void vm_eq_word(vm_t *vm)
{
  if (vm->stack.ptr < (WORD_SIZE * 2))
    // TODO: Error STACK_UNDERFLOW
    return;
  word a = vm_pop_word(vm).as_word;
  word b = vm_pop_word(vm).as_word;
  vm_push_word(vm, DWORD(a == b));
}
