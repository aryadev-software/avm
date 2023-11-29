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
#include <math.h>
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
  case ERR_STACK_UNDERFLOW:
    return "STACK_UNDERFLOW";
  case ERR_STACK_OVERFLOW:
    return "STACK_OVERFLOW";
  case ERR_CALL_STACK_UNDERFLOW:
    return "CALL_STACK_UNDERFLOW";
  case ERR_CALL_STACK_OVERFLOW:
    return "CALL_STACK_OVERFLOW";
  case ERR_INVALID_OPCODE:
    return "INVALID_OPCODE";
  case ERR_INVALID_REGISTER_BYTE:
    return "INVALID_REGISTER_BYTE";
  case ERR_INVALID_REGISTER_HWORD:
    return "INVALID_REGISTER_HWORD";
  case ERR_INVALID_REGISTER_WORD:
    return "INVALID_REGISTER_WORD";
  case ERR_INVALID_PROGRAM_ADDRESS:
    return "INVALID_PROGRAM_ADDRESS";
  case ERR_INVALID_PAGE_ADDRESS:
    return "INVALID_PAGE_ADDRESS";
  case ERR_OUT_OF_BOUNDS:
    return "OUT_OF_BOUNDS";
  case ERR_END_OF_PROGRAM:
    return "END_OF_PROGRAM";
  default:
    return "";
  }
}

err_t vm_execute(vm_t *vm)
{
  static_assert(NUMBER_OF_OPCODES == 98, "vm_execute: Out of date");
  struct Program *prog = &vm->program;
  if (prog->ptr >= prog->data->count)
    return ERR_END_OF_PROGRAM;
  inst_t instruction = prog->data->instructions[prog->ptr];

  if (OPCODE_IS_TYPE(instruction.opcode, OP_PUSH))
  {
    prog->ptr++;
    return PUSH_ROUTINES[instruction.opcode](vm, instruction.operand);
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_MOV) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_PUSH_REGISTER) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_DUP) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MALLOC) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MSET) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MGET))
  {
    err_t err =
        WORD_ROUTINES[instruction.opcode](vm, instruction.operand.as_word);
    if (err)
      return err;
    prog->ptr++;
  }
  else if (OPCODE_IS_TYPE(instruction.opcode, OP_POP))
  {
    // NOTE: We use the first register to hold the result of this pop
    data_type_t type = OPCODE_DATA_TYPE(instruction.opcode, OP_POP);
    err_t err        = ERR_OK;
    switch (type)
    {
    case DATA_TYPE_NIL:
      break;
    case DATA_TYPE_BYTE:
      err = vm_mov_byte(vm, 0);
      break;
    case DATA_TYPE_HWORD:
      err = vm_mov_hword(vm, 0);
      break;
    case DATA_TYPE_WORD:
      err = vm_mov_word(vm, 0);
      break;
    }
    if (err)
      return err;
    prog->ptr++;
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
           OPCODE_IS_TYPE(instruction.opcode, OP_PLUS) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_SUB) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MULT) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MALLOC_STACK) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MSET_STACK) ||
           OPCODE_IS_TYPE(instruction.opcode, OP_MGET_STACK) ||
           instruction.opcode == OP_MDELETE || instruction.opcode == OP_MSIZE)
  {
    err_t err = STACK_ROUTINES[instruction.opcode](vm);
    prog->ptr++;
    if (err)
      return err;
  }
  else if (instruction.opcode == OP_JUMP_ABS)
    return vm_jump(vm, instruction.operand.as_word);
  else if (instruction.opcode == OP_JUMP_STACK)
  {
    data_t ret = {0};
    // Set prog->ptr to the word on top of the stack
    err_t err = vm_pop_word(vm, &ret);
    if (err)
      return err;
    return vm_jump(vm, ret.as_word);
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
  else if (instruction.opcode == OP_CALL)
  {
    if (vm->call_stack.ptr >= vm->call_stack.max)
      return ERR_CALL_STACK_OVERFLOW;
    vm->call_stack.address_pointers[vm->call_stack.ptr++] = vm->program.ptr + 1;
    return vm_jump(vm, instruction.operand.as_word);
  }
  else if (instruction.opcode == OP_CALL_STACK)
  {
    if (vm->call_stack.ptr >= vm->call_stack.max)
      return ERR_CALL_STACK_OVERFLOW;
    vm->call_stack.address_pointers[vm->call_stack.ptr++] = vm->program.ptr + 1;
    data_t ret                                            = {0};
    err_t err = vm_pop_word(vm, &ret);
    if (err)
      return err;
    return vm_jump(vm, ret.as_word);
  }
  else if (instruction.opcode == OP_RET)
  {
    if (vm->call_stack.ptr == 0)
      return ERR_CALL_STACK_UNDERFLOW;
    return vm_jump(vm, vm->call_stack.address_pointers[--vm->call_stack.ptr]);
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
      printf("%c", datum.as_char);
      break;
    }
    case TYPE_BYTE:
      printf("0x%x", datum.as_byte);
      break;
    case TYPE_INT: {
      printf(
#if PRINT_HEX == 1
          "0x%X",
#else
          "%" PRId32,
#endif
          datum.as_int);
      break;
    }
    case TYPE_HWORD:
      printf(
#if PRINT_HEX == 1
          "0x%X",
#else
          "%" PRIu32,
#endif
          datum.as_hword);
      break;
    case TYPE_LONG: {
      printf(
#if PRINT_HEX == 1
          "0x%dX",
#else
          "%" PRId64,
#endif
          datum.as_long);
      break;
    }
    case TYPE_WORD:
      printf(
#if PRINT_HEX == 1
          "0x%lX",
#else
          "%" PRIu64,
#endif
          datum.as_word);
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
  // Setup the initial address according to the program
  program->ptr = program->data->header.start_address;
#if VERBOSE >= 1
  size_t cycles = 0;
#endif
#if VERBOSE >= 2
  registers_t prev_registers = vm->registers;
  size_t prev_sptr           = 0;
  size_t prev_pages          = 0;
  size_t prev_cptr           = 0;
#endif
  while (program->ptr < program->data->count &&
         program->data->instructions[program->ptr].opcode != OP_HALT)
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
    if (prev_cptr != vm->call_stack.ptr)
    {
      vm_print_call_stack(vm, stdout);
      prev_cptr = vm->call_stack.ptr;
      fputs("------------------------------------------------------------------"
            "----"
            "----------\n",
            stdout);
    }
    if (prev_pages != vm->heap.pages)
    {
      vm_print_heap(vm, stdout);
      prev_pages = vm->heap.pages;
      fputs("------------------------------------------------------------------"
            "----"
            "----------\n",
            stdout);
    }
    if (memcmp(&prev_registers, &vm->registers, sizeof(darr_t)) != 0)
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
  fprintf(stdout, "[%svm_execute_all%s]: Final VM state(Cycle %lu)\n",
          TERM_YELLOW, TERM_RESET, cycles);
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

void vm_load_program(vm_t *vm, prog_t *program)
{
  vm->program.ptr  = 0;
  vm->program.data = program;
}

void vm_load_registers(vm_t *vm, registers_t registers)
{
  vm->registers = registers;
}

void vm_load_heap(vm_t *vm, heap_t heap)
{
  vm->heap = heap;
}

void vm_load_call_stack(vm_t *vm, word *buffer, size_t size)
{
  vm->call_stack =
      (struct CallStack){.address_pointers = buffer, .ptr = 0, .max = size};
}

void vm_stop(vm_t *vm)
{
#if VERBOSE >= 1
  bool leaks = false;
  printf("[" TERM_YELLOW "DATA" TERM_RESET "]: Checking for leaks...\n");
  if (vm->call_stack.ptr > 0)
  {
    leaks = true;
    printf("\t[" TERM_RED "DATA" TERM_RESET "]: Call stack at %lu\n\t[" TERM_RED
           "DATA" TERM_RESET "]\n\t[" TERM_RED "DATA" TERM_RESET "]: Call "
           "stack trace:",
           vm->call_stack.ptr);
    for (size_t i = vm->call_stack.ptr; i > 0; --i)
    {
      word w = vm->call_stack.address_pointers[i - 1];
      printf("\t\t%lu: %lX", vm->call_stack.ptr - i, w);
      if (i != 1)
        printf(", ");
      printf("\n");
    }
  }
  if (vm->heap.pages > 0)
  {
    leaks       = true;
    page_t *cur = vm->heap.beg;
    size_t capacities[vm->heap.pages], total_capacity = 0;
    for (size_t i = 0; i < vm->heap.pages; ++i)
    {
      capacities[i] = cur->available;
      total_capacity += capacities[i];
    }
    printf("\t[" TERM_RED "DATA" TERM_RESET
           "]: Heap: %luB (over %lu %s) not reclaimed\n",
           total_capacity, vm->heap.pages,
           vm->heap.pages == 1 ? "page" : "pages");
    for (size_t i = 0; i < vm->heap.pages; i++)
      printf("\t\t[%lu]: %luB lost\n", i, capacities[i]);
  }
  if (vm->stack.ptr > 0)
  {
    leaks = true;
    printf("\t[" TERM_RED "DATA" TERM_RESET "]: Stack: %luB not reclaimed\n",
           vm->stack.ptr);
  }
  if (leaks)
    printf("[" TERM_RED "DATA" TERM_RESET "]: Leaks found\n");
  else
    printf("[" TERM_GREEN "DATA" TERM_RESET "]: No leaks found\n");
#endif

  free(vm->registers.data);
  free(vm->program.data);
  free(vm->stack.data);
  heap_stop(&vm->heap);
  free(vm->call_stack.address_pointers);

  vm->registers = (registers_t){0};
  vm->program   = (struct Program){0};
  vm->stack     = (struct Stack){0};
  vm->heap      = (heap_t){0};
}

void vm_print_registers(vm_t *vm, FILE *fp)
{
  registers_t reg = vm->registers;
  fprintf(
      fp,
      "Registers.used = %luB/%luH/%luW\nRegisters.available = %luB/%luH/%luW\n",
      vm->registers.used, vm->registers.used / HWORD_SIZE,
      vm->registers.used / WORD_SIZE, vm->registers.available,
      vm->registers.available / HWORD_SIZE,
      vm->registers.available / WORD_SIZE);
  fprintf(fp, "Registers.reg = [");
  for (size_t i = 0; i < ceil((long double)reg.used / WORD_SIZE); ++i)
  {
    fprintf(fp, "{%lu:%lX}", i, VM_NTH_REGISTER(reg, i));
    if (i != reg.used - 1)
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
          program.data->count, program.ptr);
  size_t beg = 0;
  if (program.ptr >= VM_PRINT_PROGRAM_EXCERPT)
  {
    fprintf(fp, "\t...\n");
    beg = program.ptr - VM_PRINT_PROGRAM_EXCERPT;
  }
  else
    beg = 0;
  size_t end = MIN(program.ptr + VM_PRINT_PROGRAM_EXCERPT, program.data->count);
  for (size_t i = beg; i < end; ++i)
  {
    fprintf(fp, "\t%lu: ", i);
    inst_print(program.data->instructions[i], fp);
    if (i == program.ptr)
      fprintf(fp, " <---");
    fprintf(fp, "\n");
  }
  if (end != program.data->count)
    fprintf(fp, "\t...\n");
  fprintf(fp, "]\n");
}

void vm_print_heap(vm_t *vm, FILE *fp)
{
  heap_t heap = vm->heap;
  fprintf(fp, "Heap.pages = %lu\nHeap.data = [", heap.pages);
  if (heap.pages == 0)
  {
    fprintf(fp, "]\n");
    return;
  }
  page_t *cur = heap.beg;
  fprintf(fp, "\n");
  for (size_t i = 0; i < heap.pages; ++i)
  {
    fprintf(fp, "\t[%lu]@%p: ", i, cur);
    if (!cur)
      fprintf(fp, "<NIL>\n");
    else
    {
      fprintf(fp, "{");
      for (size_t j = 0; j < cur->available; ++j)
      {
        if ((j % 8) == 0)
          fprintf(fp, "\n\t\t");
        fprintf(fp, "%x", cur->data[j]);
        if (j != cur->available - 1)
          fprintf(fp, ",\t");
      }
      fprintf(fp, "\n\t}\n");
      cur = cur->next;
    }
  }
  fprintf(fp, "]\n");
}

void vm_print_call_stack(vm_t *vm, FILE *fp)
{
  struct CallStack cs = vm->call_stack;
  fprintf(fp, "CallStack.max  = %lu\nCallStack.ptr  = %lu\nCallStack.data = [",
          cs.max, cs.ptr);
  if (cs.ptr == 0)
  {
    fprintf(fp, "]\n");
    return;
  }
  printf("\n");
  for (size_t i = cs.ptr; i > 0; --i)
  {
    word w = cs.address_pointers[i - 1];
    fprintf(fp, "\t%lu: %lX", cs.ptr - i, w);
    if (i != 1)
      fprintf(fp, ", ");
    fprintf(fp, "\n");
  }
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
  vm_print_call_stack(vm, fp);
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
  vm_print_heap(vm, fp);
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
  if (w >= vm->program.data->count)
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
    byte b    = bytes[HWORD_SIZE - i - 1];
    err_t err = vm_push_byte(vm, DBYTE(b));
    if (err)
      return err;
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
    byte b    = bytes[WORD_SIZE - i - 1];
    err_t err = vm_push_byte(vm, DBYTE(b));
    if (err)
      return err;
  }
  return ERR_OK;
}

err_t vm_push_byte_register(vm_t *vm, word reg)
{
  if (reg > vm->registers.used)
    return ERR_INVALID_REGISTER_BYTE;

  // Interpret each word based register as 8 byte registers
  byte b = vm->registers.data[reg];

  return vm_push_byte(vm, DBYTE(b));
}

err_t vm_push_hword_register(vm_t *vm, word reg)
{
  if (reg > (vm->registers.used / HWORD_SIZE))
    return ERR_INVALID_REGISTER_HWORD;
  // Interpret the bytes at point reg * HWORD_SIZE as an hword
  hword hw = *(hword *)(vm->registers.data + (reg * HWORD_SIZE));
  return vm_push_hword(vm, DHWORD(hw));
}

err_t vm_push_word_register(vm_t *vm, word reg)
{
  if (reg > (vm->registers.used / WORD_SIZE))
    return ERR_INVALID_REGISTER_WORD;
  return vm_push_word(vm, DWORD(VM_NTH_REGISTER(vm->registers, reg)));
}

err_t vm_mov_byte(vm_t *vm, word reg)
{
  if (reg >= vm->registers.used)
  {
    // Expand capacity
    darr_ensure_capacity(&vm->registers, reg - vm->registers.used);
    vm->registers.used = MAX(vm->registers.used, reg + 1);
  }
  data_t ret = {0};
  err_t err  = vm_pop_byte(vm, &ret);
  if (err)
    return err;
  vm->registers.data[reg] = ret.as_byte;
  return ERR_OK;
}

err_t vm_mov_hword(vm_t *vm, word reg)
{
  if (reg >= (vm->registers.used / HWORD_SIZE))
  {
    // Expand capacity till we can ensure that this is a valid
    // register to use

    // Number of hwords needed ontop of what is allocated:
    const size_t hwords = (reg - (vm->registers.used / HWORD_SIZE));
    // Number of bytes needed ontop of what is allocated
    const size_t diff = (hwords + 1) * HWORD_SIZE;

    darr_ensure_capacity(&vm->registers, diff);
    vm->registers.used = MAX(vm->registers.used, (reg + 1) * HWORD_SIZE);
  }
  data_t ret = {0};
  err_t err  = vm_pop_hword(vm, &ret);
  if (err)
    return err;
  // Here we treat vm->registers as a set of hwords
  hword *hword_ptr = (hword *)(vm->registers.data + (reg * HWORD_SIZE));
  *hword_ptr       = ret.as_hword;
  return ERR_OK;
}

err_t vm_mov_word(vm_t *vm, word reg)
{
  if (reg >= (vm->registers.used / WORD_SIZE))
  {
    // Number of hwords needed ontop of what is allocated:
    const size_t words = (reg - (vm->registers.used / WORD_SIZE));
    // Number of bytes needed ontop of what is allocated
    const size_t diff = (words + 1) * WORD_SIZE;

    darr_ensure_capacity(&vm->registers, diff);
    vm->registers.used = MAX(vm->registers.used, (reg + 1) * WORD_SIZE);
  }
  else if (vm->stack.ptr < sizeof(word))
    return ERR_STACK_UNDERFLOW;
  data_t ret = {0};
  err_t err  = vm_pop_word(vm, &ret);
  if (err)
    return err;
  ((word *)(vm->registers.data))[reg] = ret.as_word;
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

err_t vm_malloc_byte(vm_t *vm, word n)
{
  page_t *page = heap_allocate(&vm->heap, n);
  return vm_push_word(vm, DWORD((word)page));
}

err_t vm_malloc_hword(vm_t *vm, word n)
{
  page_t *page = heap_allocate(&vm->heap, n * HWORD_SIZE);
  return vm_push_word(vm, DWORD((word)page));
}

err_t vm_malloc_word(vm_t *vm, word n)
{
  page_t *page = heap_allocate(&vm->heap, n * WORD_SIZE);
  return vm_push_word(vm, DWORD((word)page));
}

err_t vm_mset_byte(vm_t *vm, word nth)
{
  // Stack layout should be [BYTE, PTR]
  data_t byte = {0};
  err_t err   = vm_pop_byte(vm, &byte);
  if (err)
    return err;
  data_t ptr = {0};
  err        = vm_pop_word(vm, &ptr);
  if (err)
    return err;

  page_t *page = (page_t *)ptr.as_word;
  if (nth >= page->available)
    return ERR_OUT_OF_BOUNDS;
  page->data[nth] = byte.as_byte;

  return ERR_OK;
}

err_t vm_mset_hword(vm_t *vm, word nth)
{
  // Stack layout should be [HWORD, PTR]
  data_t byte = {0};
  err_t err   = vm_pop_hword(vm, &byte);
  if (err)
    return err;
  data_t ptr = {0};
  err        = vm_pop_word(vm, &ptr);
  if (err)
    return err;

  page_t *page = (page_t *)ptr.as_word;
  if (nth >= (page->available / HWORD_SIZE))
    return ERR_OUT_OF_BOUNDS;
  ((hword *)page->data)[nth] = byte.as_hword;

  return ERR_OK;
}

err_t vm_mset_word(vm_t *vm, word nth)
{
  // Stack layout should be [WORD, PTR]
  data_t byte = {0};
  err_t err   = vm_pop_word(vm, &byte);
  if (err)
    return err;
  data_t ptr = {0};
  err        = vm_pop_word(vm, &ptr);
  if (err)
    return err;

  page_t *page = (page_t *)ptr.as_word;
  if (nth >= (page->available / WORD_SIZE))
    return ERR_OUT_OF_BOUNDS;
  ((word *)page->data)[nth] = byte.as_word;

  return ERR_OK;
}

err_t vm_mget_byte(vm_t *vm, word n)
{
  // Stack layout should be [PTR]
  data_t ptr = {0};
  err_t err  = vm_pop_word(vm, &ptr);
  if (err)
    return err;
  page_t *page = (page_t *)ptr.as_word;
  if (n >= page->available)
    return ERR_OUT_OF_BOUNDS;
  return vm_push_byte(vm, DBYTE(page->data[n]));
}

err_t vm_mget_hword(vm_t *vm, word n)
{
  // Stack layout should be [PTR]
  data_t ptr = {0};
  err_t err  = vm_pop_word(vm, &ptr);
  if (err)
    return err;
  page_t *page = (page_t *)ptr.as_word;
  if (n >= (page->available / HWORD_SIZE))
    return ERR_OUT_OF_BOUNDS;
  return vm_push_hword(vm, DHWORD(((hword *)page->data)[n]));
}

err_t vm_mget_word(vm_t *vm, word n)
{
  // Stack layout should be [PTR]
  data_t ptr = {0};
  err_t err  = vm_pop_word(vm, &ptr);
  if (err)
    return err;
  printf("%lx\n", ptr.as_word);
  page_t *page = (page_t *)ptr.as_word;
  if (n >= (page->available / WORD_SIZE))
    return ERR_OUT_OF_BOUNDS;
  return vm_push_word(vm, DWORD(((word *)page->data)[n]));
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

// TODO: rename this to something more appropriate
#define VM_MEMORY_STACK_CONSTR(ACTION, TYPE)    \
  err_t vm_##ACTION##_stack_##TYPE(vm_t *vm)    \
  {                                             \
    data_t n  = {0};                            \
    err_t err = vm_pop_word(vm, &n);            \
    if (err)                                    \
      return err;                               \
    return vm_##ACTION##_##TYPE(vm, n.as_word); \
  }

VM_MEMORY_STACK_CONSTR(malloc, byte)
VM_MEMORY_STACK_CONSTR(malloc, hword)
VM_MEMORY_STACK_CONSTR(malloc, word)
VM_MEMORY_STACK_CONSTR(mset, byte)
VM_MEMORY_STACK_CONSTR(mset, hword)
VM_MEMORY_STACK_CONSTR(mset, word)
VM_MEMORY_STACK_CONSTR(mget, byte)
VM_MEMORY_STACK_CONSTR(mget, hword)
VM_MEMORY_STACK_CONSTR(mget, word)

err_t vm_mdelete(vm_t *vm)
{
  data_t ptr = {0};
  err_t err  = vm_pop_word(vm, &ptr);
  if (err)
    return err;
  page_t *page = (page_t *)ptr.as_word;
  bool done    = heap_free_page(&vm->heap, page);
  if (!done)
    return ERR_INVALID_PAGE_ADDRESS;
  return ERR_OK;
}

err_t vm_msize(vm_t *vm)
{
  data_t ptr = {0};
  err_t err  = vm_pop_word(vm, &ptr);
  if (err)
    return err;
  page_t *page = (page_t *)ptr.as_word;
  return vm_push_word(vm, DWORD(page->available));
}

// TODO: rename this to something more appropriate
#define VM_NOT_TYPE(TYPEL, TYPEU)                        \
  err_t vm_not_##TYPEL(vm_t *vm)                         \
  {                                                      \
    data_t a  = {0};                                     \
    err_t err = vm_pop_##TYPEL(vm, &a);                  \
    if (err)                                             \
      return err;                                        \
    return vm_push_##TYPEL(vm, D##TYPEU(!a.as_##TYPEL)); \
  }

VM_NOT_TYPE(byte, BYTE)
VM_NOT_TYPE(hword, HWORD)
VM_NOT_TYPE(word, WORD)

// TODO: rename this to something more appropriate
#define VM_SAME_TYPE(COMPNAME, COMP, TYPEL, TYPEU)                        \
  err_t vm_##COMPNAME##_##TYPEL(vm_t *vm)                                 \
  {                                                                       \
    data_t a = {0}, b = {0};                                              \
    err_t err = vm_pop_##TYPEL(vm, &a);                                   \
    if (err)                                                              \
      return err;                                                         \
    err = vm_pop_##TYPEL(vm, &b);                                         \
    if (err)                                                              \
      return err;                                                         \
    return vm_push_##TYPEL(vm, D##TYPEU(a.as_##TYPEL COMP b.as_##TYPEL)); \
  }

// TODO: rename this to something more appropriate
#define VM_COMPARATOR_TYPE(COMPNAME, COMP, TYPEL, GETL)           \
  err_t vm_##COMPNAME##_##GETL(vm_t *vm)                          \
  {                                                               \
    data_t a = {0}, b = {0};                                      \
    err_t err = vm_pop_##TYPEL(vm, &a);                           \
    if (err)                                                      \
      return err;                                                 \
    err = vm_pop_##TYPEL(vm, &b);                                 \
    if (err)                                                      \
      return err;                                                 \
    return vm_push_byte(vm, DBYTE(b.as_##GETL COMP a.as_##GETL)); \
  }

VM_SAME_TYPE(or, |, byte, BYTE)
VM_SAME_TYPE(or, |, hword, HWORD)
VM_SAME_TYPE(or, |, word, WORD)
VM_SAME_TYPE(and, &, byte, BYTE)
VM_SAME_TYPE(and, &, hword, HWORD)
VM_SAME_TYPE(and, &, word, WORD)
VM_SAME_TYPE(xor, ^, byte, BYTE)
VM_SAME_TYPE(xor, ^, hword, HWORD)
VM_SAME_TYPE(xor, ^, word, WORD)

VM_SAME_TYPE(plus, +, byte, BYTE)
VM_SAME_TYPE(plus, +, hword, HWORD)
VM_SAME_TYPE(plus, +, word, WORD)

VM_SAME_TYPE(sub, -, byte, BYTE)
VM_SAME_TYPE(sub, -, hword, HWORD)
VM_SAME_TYPE(sub, -, word, WORD)

VM_SAME_TYPE(mult, *, byte, BYTE)
VM_SAME_TYPE(mult, *, hword, HWORD)
VM_SAME_TYPE(mult, *, word, WORD)

VM_COMPARATOR_TYPE(eq, ==, byte, byte)
VM_COMPARATOR_TYPE(eq, ==, byte, char)
VM_COMPARATOR_TYPE(eq, ==, hword, hword)
VM_COMPARATOR_TYPE(eq, ==, hword, int)
VM_COMPARATOR_TYPE(eq, ==, word, word)
VM_COMPARATOR_TYPE(eq, ==, word, long)

VM_COMPARATOR_TYPE(lt, <, byte, byte)
VM_COMPARATOR_TYPE(lt, <, byte, char)
VM_COMPARATOR_TYPE(lt, <, hword, hword)
VM_COMPARATOR_TYPE(lt, <, hword, int)
VM_COMPARATOR_TYPE(lt, <, word, word)
VM_COMPARATOR_TYPE(lt, <, word, long)

VM_COMPARATOR_TYPE(lte, <=, byte, byte)
VM_COMPARATOR_TYPE(lte, <=, byte, char)
VM_COMPARATOR_TYPE(lte, <=, hword, hword)
VM_COMPARATOR_TYPE(lte, <=, hword, int)
VM_COMPARATOR_TYPE(lte, <=, word, word)
VM_COMPARATOR_TYPE(lte, <=, word, long)

VM_COMPARATOR_TYPE(gt, >, byte, byte)
VM_COMPARATOR_TYPE(gt, >, byte, char)
VM_COMPARATOR_TYPE(gt, >, hword, hword)
VM_COMPARATOR_TYPE(gt, >, hword, int)
VM_COMPARATOR_TYPE(gt, >, word, word)
VM_COMPARATOR_TYPE(gt, >, word, long)

VM_COMPARATOR_TYPE(gte, >=, byte, byte)
VM_COMPARATOR_TYPE(gte, >=, byte, char)
VM_COMPARATOR_TYPE(gte, >=, hword, hword)
VM_COMPARATOR_TYPE(gte, >=, hword, int)
VM_COMPARATOR_TYPE(gte, >=, word, word)
VM_COMPARATOR_TYPE(gte, >=, word, long)
