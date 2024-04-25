/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2024-04-25
 * Author: Aryadev Chavali
 * Description: Virtual machine data structures and some helpers
 */

#include <math.h>
#include <stdio.h>

#include "./struct.h"

void vm_load_stack(vm_t *vm, byte_t *bytes, size_t size)
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

void vm_load_call_stack(vm_t *vm, word_t *buffer, size_t size)
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
      word_t w = vm->call_stack.address_pointers[i - 1];
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
    byte_t b = stack.data[i - 1];
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
  const size_t count     = program.data->count;
  fprintf(fp,
          "Program.max          = %lu\nProgram.ptr          = "
          "%lu\nProgram.instructions = [\n",
          count, program.ptr);
  size_t beg = 0;
  if (program.ptr >= VM_PRINT_PROGRAM_EXCERPT)
  {
    fprintf(fp, "\t...\n");
    beg = program.ptr - VM_PRINT_PROGRAM_EXCERPT;
  }
  else
    beg = 0;
  size_t end = MIN(program.ptr + VM_PRINT_PROGRAM_EXCERPT, count);
  for (size_t i = beg; i < end; ++i)
  {
    fprintf(fp, "\t%lu: ", i);
    inst_print(program.data->instructions[i], fp);
    if (i == program.ptr)
      fprintf(fp, " <---");
    fprintf(fp, "\n");
  }
  if (end != count)
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
    word_t w = cs.address_pointers[i - 1];
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
