/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Virtual machine implementation
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./runtime.h"

const char *err_as_cstr(err_t err)
{
  switch (err)
  {
  case ERR_OK:
    return "OK";
    break;
  case ERR_STACK_UNDERFLOW:
    return "STACK_UNDERFLOW";
    break;
  case ERR_STACK_OVERFLOW:
    return "STACK_OVERFLOW";
    break;
  case ERR_INVALID_OPCODE:
    return "INVALID_OPCODE";
    break;
  case ERR_INVALID_REGISTER_BYTE:
    return "INVALID_REGISTER_BYTE";
    break;
  case ERR_INVALID_REGISTER_HWORD:
    return "INVALID_REGISTER_HWORD";
    break;
  case ERR_INVALID_REGISTER_WORD:
    return "INVALID_REGISTER_WORD";
    break;
  case ERR_INVALID_PROGRAM_ADDRESS:
    return "INVALID_PROGRAM_ADDRESS";
  case ERR_END_OF_PROGRAM:
    return "END_OF_PROGRAM";
    break;
  default:
    return "";
  }
}

err_t vm_execute(vm_t *vm)
{
  static_assert(NUMBER_OF_OPCODES == 73, "vm_execute: Out of date");
  struct Program *prog = &vm->program;
  if (prog->ptr >= prog->max)
    return ERR_END_OF_PROGRAM;
  inst_t instruction = prog->instructions[prog->ptr];

  if (OPCODE_IS_TYPE(instruction.opcode, OP_PUSH))
  {
    prog->ptr++;
    return PUSH_ROUTINES[instruction.opcode](vm, instruction.operand);
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_MOV) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_PUSH_REGISTER))
  {
    prog->ptr++;
    return REG_ROUTINES[instruction.opcode](vm, instruction.operand.as_byte);
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_POP))
  {
    // NOTE: We use the first register to hold the result of this pop
    data_type_t type = OPCODE_DATA_TYPE(instruction.opcode, OP_POP);
    prog->ptr++;
    switch (type)
    {
    case DATA_TYPE_NIL:
      break;
    case DATA_TYPE_BYTE:
      return vm_mov_byte(vm, 0);
      break;
    case DATA_TYPE_HWORD:
      return vm_mov_hword(vm, 0);
      break;
    case DATA_TYPE_WORD:
      return vm_mov_word(vm, 0);
      break;
    }
    return ERR_OK;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_DUP))
  {
    prog->ptr++;
    return DUP_ROUTINES[instruction.opcode](vm, instruction.operand.as_word);
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_NOT) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_OR) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_AND) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_XOR) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_EQ) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_LT) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_LTE) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_GT) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_GTE) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_PLUS))
  {
    prog->ptr++;
    return STACK_ROUTINES[instruction.opcode](vm);
  }
  else if (instruction.opcode == OP_JUMP_ABS)
    return vm_jump(vm, instruction.operand.as_word);
  else if (instruction.opcode == OP_JUMP_STACK)
  {
    // Set prog->ptr to the word on top of the stack
    data_t ret = {0};
    err_t err  = vm_pop_word(vm, &ret);
    if (err)
      return err;
    return vm_jump(vm, ret.as_word);
  }
  else if (instruction.opcode == OP_JUMP_REGISTER)
  {
    if (instruction.operand.as_byte >= 8)
      return ERR_INVALID_REGISTER_WORD;
    word addr = vm->registers.reg[instruction.operand.as_byte];
    return vm_jump(vm, addr);
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_JUMP_IF))
  {
    data_t datum = {0};
    err_t err    = ERR_OK;
    if (instruction.opcode == OP_JUMP_IF_BYTE)
      err = vm_pop_byte(vm, &datum);
    else if (instruction.opcode == OP_JUMP_IF_HWORD)
      err = vm_pop_hword(vm, &datum);
    else if (instruction.opcode == OP_JUMP_IF_WORD)
      err = vm_pop_word(vm, &datum);

    if (err)
      return err;

    // If datum != 0 then jump, else go to the next instruction
    if (datum.as_word != 0)
      return vm_jump(vm, instruction.operand.as_word);
    else
      ++prog->ptr;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_PRINT))
  {
    data_t datum = {0};
    enum
    {
      TYPE_BYTE,
      TYPE_CHAR,
      TYPE_INT,
      TYPE_HWORD,
      TYPE_LONG,
      TYPE_WORD
    } print_type;
    err_t err = ERR_OK;
    if (instruction.opcode == OP_PRINT_BYTE ||
        instruction.opcode == OP_PRINT_CHAR)
    {
      print_type = instruction.opcode == OP_PRINT_BYTE ? TYPE_BYTE : TYPE_CHAR;
      err        = vm_pop_byte(vm, &datum);
    }
    else if (instruction.opcode == OP_PRINT_HWORD ||
             instruction.opcode == OP_PRINT_INT)
    {
      print_type = instruction.opcode == OP_PRINT_HWORD ? TYPE_HWORD : TYPE_INT;
      err        = vm_pop_hword(vm, &datum);
    }
    else if (instruction.opcode == OP_PRINT_WORD ||
             instruction.opcode == OP_PRINT_LONG)
    {
      print_type = instruction.opcode == OP_PRINT_WORD ? TYPE_WORD : TYPE_LONG;
      err        = vm_pop_word(vm, &datum);
    }

    if (err)
      return err;

    switch (print_type)
    {
    case TYPE_CHAR: {
      char c = 0;
      memcpy(&c, &datum.as_byte, 1);
      printf("%c", c);
      break;
    }
    case TYPE_BYTE:
      printf("0x%x", datum.as_byte);
      break;
    case TYPE_INT: {
      s_hword i = 0;
      memcpy(&i, &datum.as_hword, HWORD_SIZE);
      printf("%" PRId32, i);
      break;
    }
    case TYPE_HWORD:
      printf("%" PRIu32, datum.as_hword);
      break;
    case TYPE_LONG: {
      s_word i = 0;
      memcpy(&i, &datum.as_word, WORD_SIZE);
      printf("%" PRId64, i);
      break;
    }
    case TYPE_WORD:
      printf("%" PRIu64, datum.as_word);
      break;
    }

    prog->ptr++;
  }
  else if (instruction.opcode == OP_HALT)
  {
    // Do nothing here.  Should be caught by callers of vm_execute
  }
  else
    return ERR_INVALID_OPCODE;
  return ERR_OK;
}

err_t vm_execute_all(vm_t *vm)
{
  struct Program *program = &vm->program;
  err_t err               = ERR_OK;
#if VERBOSE >= 1
  size_t cycles = 0;
#endif
#if VERBOSE >= 2
  struct Registers prev_registers = vm->registers;
  size_t prev_sptr                = 0;
#endif
  while (program->instructions[program->ptr].opcode != OP_HALT &&
         program->ptr < program->max)
  {
#if VERBOSE >= 2
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
#endif
#if VERBOSE >= 1
    ++cycles;
#endif
    err = vm_execute(vm);
    if (err)
      return err;
  }

#if VERBOSE >= 1
  fprintf(stdout, "[vm_execute_all]: Final VM state(Cycle %lu)\n", cycles);
  vm_print_all(vm, stdout);
#endif
  return err;
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
    inst_print(program.instructions[i], fp);
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
  vm_print_program(vm, fp);
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
}

err_t vm_jump(vm_t *vm, word w)
{
  if (w >= vm->program.max)
    return ERR_INVALID_PROGRAM_ADDRESS;
  vm->program.ptr = w;
  return ERR_OK;
}

err_t vm_push_byte(vm_t *vm, data_t b)
{
  if (vm->stack.ptr >= vm->stack.max)
    return ERR_STACK_OVERFLOW;
  vm->stack.data[vm->stack.ptr++] = b.as_byte;
  return ERR_OK;
}

err_t vm_push_hword(vm_t *vm, data_t f)
{
  if (vm->stack.ptr + HWORD_SIZE >= vm->stack.max)
    return ERR_STACK_OVERFLOW;
  byte bytes[HWORD_SIZE] = {0};
  convert_hword_to_bytes(f.as_hword, bytes);
  for (size_t i = 0; i < HWORD_SIZE; ++i)
  {
    byte b = bytes[HWORD_SIZE - i - 1];
    vm_push_byte(vm, DBYTE(b));
  }
  return ERR_OK;
}

err_t vm_push_word(vm_t *vm, data_t w)
{
  if (vm->stack.ptr + WORD_SIZE >= vm->stack.max)
    return ERR_STACK_OVERFLOW;
  byte bytes[WORD_SIZE] = {0};
  convert_word_to_bytes(w.as_word, bytes);
  for (size_t i = 0; i < WORD_SIZE; ++i)
  {
    byte b = bytes[WORD_SIZE - i - 1];
    vm_push_byte(vm, DBYTE(b));
  }
  return ERR_OK;
}

err_t vm_push_byte_register(vm_t *vm, byte reg)
{
  if (reg >= VM_REGISTERS * 8)
    return ERR_INVALID_REGISTER_BYTE;

  // Interpret each word based register as 8 byte registers
  byte b = WORD_NTH_BYTE(vm->registers.reg[reg / 8], reg % 8);

  return vm_push_byte(vm, DBYTE(b));
}

err_t vm_push_hword_register(vm_t *vm, byte reg)
{
  if (reg >= VM_REGISTERS * 2)
    return ERR_INVALID_REGISTER_HWORD;
  else if (vm->stack.ptr >= vm->stack.max)
    return ERR_STACK_OVERFLOW;
  // Interpret each word based register as 2 hword registers
  hword hw = WORD_NTH_HWORD(vm->registers.reg[reg / 2], reg % 2);
  return vm_push_hword(vm, DHWORD(hw));
}

err_t vm_push_word_register(vm_t *vm, byte reg)
{
  if (reg >= VM_REGISTERS)
    return ERR_INVALID_REGISTER_WORD;
  else if (vm->stack.ptr >= vm->stack.max)
    return ERR_STACK_OVERFLOW;
  return vm_push_word(vm, DWORD(vm->registers.reg[reg]));
}

err_t vm_mov_byte(vm_t *vm, byte reg)
{
  if (reg >= (VM_REGISTERS * 8))
    return ERR_INVALID_REGISTER_BYTE;
  data_t ret = {0};
  err_t err  = vm_pop_byte(vm, &ret);
  if (err)
    return err;
  word *reg_ptr = &vm->registers.reg[reg / 8];
  size_t shift  = (reg % 8) * 8;
  // This resets the bits in the specific byte register
  *reg_ptr = *reg_ptr & ~(0xFF << shift);
  // This sets the bits
  *reg_ptr = (*reg_ptr) | (ret.as_word << shift);
  return ERR_OK;
}

err_t vm_mov_hword(vm_t *vm, byte reg)
{
  if (reg >= (VM_REGISTERS * 2))
    return ERR_INVALID_REGISTER_HWORD;
  else if (vm->stack.ptr < sizeof(f64))
    return ERR_STACK_UNDERFLOW;
  data_t ret = {0};
  err_t err  = vm_pop_hword(vm, &ret);
  if (err)
    return err;
  word *reg_ptr = &vm->registers.reg[reg / 2];
  size_t shift  = (reg % 2) * 2;
  // This resets the bits in the specific hword register
  *reg_ptr = *reg_ptr & ~(0xFFFFFFFF << shift);
  // This sets the bits
  *reg_ptr = (*reg_ptr) | (ret.as_word << shift);
  return ERR_OK;
}

err_t vm_mov_word(vm_t *vm, byte reg)
{
  if (reg >= VM_REGISTERS)
    return ERR_INVALID_REGISTER_WORD;
  else if (vm->stack.ptr < sizeof(word))
    return ERR_STACK_UNDERFLOW;
  data_t ret = {0};
  err_t err  = vm_pop_word(vm, &ret);
  if (err)
    return err;
  vm->registers.reg[reg] = ret.as_word;
  return ERR_OK;
}

err_t vm_dup_byte(vm_t *vm, word w)
{
  if (vm->stack.ptr < w + 1)
    return ERR_STACK_UNDERFLOW;
  return vm_push_byte(vm, DBYTE(vm->stack.data[vm->stack.ptr - 1 - w]));
}

err_t vm_dup_hword(vm_t *vm, word w)
{
  if (vm->stack.ptr < HWORD_SIZE * (w + 1))
    return ERR_STACK_UNDERFLOW;
  byte bytes[HWORD_SIZE] = {0};
  for (size_t i = 0; i < HWORD_SIZE; ++i)
    bytes[HWORD_SIZE - i - 1] =
        vm->stack.data[vm->stack.ptr - (HWORD_SIZE * (w + 1)) + i];
  return vm_push_hword(vm, DHWORD(convert_bytes_to_hword(bytes)));
}

err_t vm_dup_word(vm_t *vm, word w)
{
  if (vm->stack.ptr < WORD_SIZE * (w + 1))
    return ERR_STACK_UNDERFLOW;
  byte bytes[WORD_SIZE] = {0};
  for (size_t i = 0; i < WORD_SIZE; ++i)
    bytes[WORD_SIZE - i - 1] =
        vm->stack.data[vm->stack.ptr - (WORD_SIZE * (w + 1)) + i];
  return vm_push_word(vm, DWORD(convert_bytes_to_word(bytes)));
}

err_t vm_pop_byte(vm_t *vm, data_t *ret)
{
  if (vm->stack.ptr == 0)
    return ERR_STACK_UNDERFLOW;
  *ret = DBYTE(vm->stack.data[--vm->stack.ptr]);
  return ERR_OK;
}

err_t vm_pop_hword(vm_t *vm, data_t *ret)
{
  if (vm->stack.ptr < HWORD_SIZE)
    return ERR_STACK_UNDERFLOW;
  byte bytes[HWORD_SIZE] = {0};
  for (size_t i = 0; i < HWORD_SIZE; ++i)
  {
    data_t b = {0};
    vm_pop_byte(vm, &b);
    bytes[i] = b.as_byte;
  }
  *ret = DWORD(convert_bytes_to_hword(bytes));
  return ERR_OK;
}

err_t vm_pop_word(vm_t *vm, data_t *ret)
{
  if (vm->stack.ptr < WORD_SIZE)
    return ERR_STACK_UNDERFLOW;
  byte bytes[WORD_SIZE] = {0};
  for (size_t i = 0; i < WORD_SIZE; ++i)
  {
    data_t b = {0};
    vm_pop_byte(vm, &b);
    bytes[i] = b.as_byte;
  }
  *ret = DWORD(convert_bytes_to_word(bytes));
  return ERR_OK;
}

err_t vm_not_byte(vm_t *vm)
{
  data_t a  = {0};
  err_t err = vm_pop_byte(vm, &a);
  if (err)
    return err;
  return vm_push_byte(vm, DBYTE(!a.as_byte));
}

err_t vm_not_hword(vm_t *vm)
{
  data_t a  = {0};
  err_t err = vm_pop_hword(vm, &a);
  if (err)
    return err;
  return vm_push_hword(vm, DHWORD(!a.as_hword));
}

err_t vm_not_word(vm_t *vm)
{
  data_t a  = {0};
  err_t err = vm_pop_word(vm, &a);
  if (err)
    return err;
  return vm_push_word(vm, DWORD(!a.as_word));
}

err_t vm_or_byte(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_byte(vm, &a);
  if (err)
    return err;
  err = vm_pop_byte(vm, &b);
  if (err)
    return err;
  return vm_push_byte(vm, DBYTE(a.as_byte | b.as_byte));
}

err_t vm_or_hword(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_hword(vm, &a);
  if (err)
    return err;
  err = vm_pop_hword(vm, &b);
  if (err)
    return err;
  return vm_push_hword(vm, DHWORD(a.as_hword | b.as_hword));
}

err_t vm_or_word(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_word(vm, &a);
  if (err)
    return err;
  err = vm_pop_word(vm, &b);
  if (err)
    return err;
  return vm_push_word(vm, DWORD(a.as_word | b.as_word));
}

err_t vm_and_byte(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_byte(vm, &a);
  if (err)
    return err;
  err = vm_pop_byte(vm, &b);
  if (err)
    return err;
  return vm_push_byte(vm, DBYTE(a.as_byte & b.as_byte));
}

err_t vm_and_hword(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_hword(vm, &a);
  if (err)
    return err;
  err = vm_pop_hword(vm, &b);
  if (err)
    return err;
  return vm_push_hword(vm, DHWORD(a.as_hword & b.as_hword));
}

err_t vm_and_word(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_word(vm, &a);
  if (err)
    return err;
  err = vm_pop_word(vm, &b);
  if (err)
    return err;
  return vm_push_word(vm, DWORD(a.as_word & b.as_word));
}

err_t vm_xor_byte(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_byte(vm, &a);
  if (err)
    return err;
  err = vm_pop_byte(vm, &b);
  if (err)
    return err;
  return vm_push_byte(vm, DBYTE(a.as_byte ^ b.as_byte));
}

err_t vm_xor_hword(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_hword(vm, &a);
  if (err)
    return err;
  err = vm_pop_hword(vm, &b);
  if (err)
    return err;
  return vm_push_hword(vm, DHWORD(a.as_hword ^ b.as_hword));
}

err_t vm_xor_word(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_word(vm, &a);
  if (err)
    return err;
  err = vm_pop_word(vm, &b);
  if (err)
    return err;
  return vm_push_word(vm, DWORD(a.as_word ^ b.as_word));
}

err_t vm_eq_byte(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_byte(vm, &a);
  if (err)
    return err;
  err = vm_pop_byte(vm, &b);
  if (err)
    return err;
  return vm_push_byte(vm, DBYTE(a.as_byte == b.as_byte));
}

err_t vm_eq_char(vm_t *vm)
{
  return vm_eq_byte(vm);
}

err_t vm_eq_hword(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_hword(vm, &a);
  if (err)
    return err;
  err = vm_pop_hword(vm, &b);
  if (err)
    return err;
  return vm_push_byte(vm, DBYTE(a.as_hword == b.as_hword));
}

err_t vm_eq_int(vm_t *vm)
{
  return vm_eq_hword(vm);
}

err_t vm_eq_long(vm_t *vm)
{
  return vm_eq_word(vm);
}

err_t vm_eq_word(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_word(vm, &a);
  if (err)
    return err;
  err = vm_pop_word(vm, &b);
  if (err)
    return err;
  return vm_push_byte(vm, DBYTE(a.as_word == b.as_word));
}

err_t vm_lt_byte(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lt_char(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lt_int(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lt_hword(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lt_long(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lt_word(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lte_byte(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lte_char(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lte_int(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lte_hword(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lte_long(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_lte_word(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gt_byte(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gt_char(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gt_int(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gt_hword(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gt_long(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gt_word(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gte_byte(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gte_char(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gte_int(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gte_hword(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gte_long(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_gte_word(vm_t *vm)
{
  (void)vm;
  return ERR_END_OF_PROGRAM;
}

err_t vm_plus_byte(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_byte(vm, &a);
  if (err)
    return err;
  err = vm_pop_byte(vm, &b);
  if (err)
    return err;
  return vm_push_byte(vm, DBYTE(a.as_byte + b.as_byte));
}

err_t vm_plus_hword(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_hword(vm, &a);
  if (err)
    return err;
  err = vm_pop_hword(vm, &b);
  if (err)
    return err;
  return vm_push_hword(vm, DHWORD(a.as_hword + b.as_hword));
}

err_t vm_plus_word(vm_t *vm)
{
  data_t a = {0}, b = {0};
  err_t err = vm_pop_word(vm, &a);
  if (err)
    return err;
  err = vm_pop_word(vm, &b);
  if (err)
    return err;
  return vm_push_word(vm, DWORD(a.as_word + b.as_word));
}
