/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Entrypoint to program
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

typedef uint8_t byte;
typedef u64 word;

#define WORD_SIZE sizeof(word)

typedef union
{
  byte as_byte;
  word as_word;
} data_t;

#define DBYTE(BYTE) ((data_t){.as_byte = (BYTE)})
#define DWORD(WORD) ((data_t){.as_word = (WORD)})

#define VM_STACK_MAX 1024

typedef struct
{
  struct Stack
  {
    byte data[VM_STACK_MAX];
    word pointer;
  } stack;
} vm_t;

void vm_push_byte(vm_t *vm, data_t b)
{
  if (vm->stack.pointer >= VM_STACK_MAX)
    // TODO: Error STACK_OVERFLOW
    return;
  vm->stack.data[vm->stack.pointer++] = b.as_byte;
}

void vm_push_word(vm_t *vm, data_t w)
{
  // NOTE: Relies on sizeof measuring in bytes
  if (vm->stack.pointer + WORD_SIZE >= VM_STACK_MAX)
    // TODO: Error STACK_OVERFLOW
    return;
  memcpy(vm->stack.data + vm->stack.pointer, &w.as_word, WORD_SIZE);
  vm->stack.pointer += WORD_SIZE;
}

byte vm_pop_byte(vm_t *vm)
{
  if (vm->stack.pointer == 0)
    // TODO: Error STACK_UNDERFLOW
    return 0;
  return vm->stack.data[--vm->stack.pointer];
}

word vm_pop_word(vm_t *vm)
{
  if (vm->stack.pointer < WORD_SIZE)
    // TODO: Error STACK_UNDERFLOW
    return 0;
  word w = 0;
  memcpy(&w, vm->stack.data + vm->stack.pointer - WORD_SIZE, WORD_SIZE);
  vm->stack.pointer -= WORD_SIZE;
  return w;
}

int main(void)
{
  puts("Hello, world!");
  return 0;
}
