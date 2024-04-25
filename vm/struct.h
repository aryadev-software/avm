/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2024-04-25
 * Author: Aryadev Chavali
 * Description: Virtual machine data structures and some helpers
 */

#ifndef STRUCT_H
#define STRUCT_H

#include <lib/darr.h>
#include <lib/heap.h>
#include <lib/inst.h>

typedef darr_t registers_t;
#define VM_NTH_REGISTER(REGISTERS, N)     (((word *)((REGISTERS).data))[N])
#define VM_REGISTERS_AVAILABLE(REGISTERS) (((REGISTERS).available) / WORD_SIZE)

struct Stack
{
  byte_t *data;
  size_t ptr, max;
};

struct Program
{
  prog_t *data;
  word ptr;
};

struct CallStack
{
  word *address_pointers;
  size_t ptr, max;
};

typedef struct
{
  registers_t registers;
  struct Stack stack;
  heap_t heap;

  struct CallStack call_stack;
  struct Program program;
} vm_t;

// Start and stop
void vm_load_stack(vm_t *, byte_t *, size_t);
void vm_load_registers(vm_t *, registers_t);
void vm_load_heap(vm_t *, heap_t);
void vm_load_program(vm_t *, prog_t *);
void vm_load_call_stack(vm_t *, word *, size_t);
void vm_stop(vm_t *);

// Printing the VM
#define VM_PRINT_PROGRAM_EXCERPT 5
void vm_print_registers(vm_t *, FILE *);
void vm_print_stack(vm_t *, FILE *);
void vm_print_program(vm_t *, FILE *);
void vm_print_heap(vm_t *, FILE *);
void vm_print_call_stack(vm_t *, FILE *);
void vm_print_all(vm_t *, FILE *);

#endif
