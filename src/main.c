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

typedef union
{
  byte as_byte;
  word as_word;
  f64 as_float;
} data_t;

#define DBYTE(BYTE)   ((data_t){.as_byte = (BYTE)})
#define DWORD(WORD)   ((data_t){.as_word = (WORD)})
#define DFLOAT(FLOAT) ((data_t){.as_float = (FLOAT)})

#define WORD_SIZE  sizeof(word)
#define FLOAT_SIZE sizeof(f64)

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
  if (vm->stack.pointer + WORD_SIZE >= VM_STACK_MAX)
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
  if (vm->stack.pointer + FLOAT_SIZE >= VM_STACK_MAX)
    // TODO: Error STACK_OVERFLOW
    return;
  // TODO: Make this machine independent (encode IEEE754 floats
  // yourself?)
  memcpy(vm->stack.data + vm->stack.pointer, &f.as_float, FLOAT_SIZE);
  vm->stack.pointer += FLOAT_SIZE;
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
  if (vm->stack.pointer < FLOAT_SIZE)
    // TODO: Error STACK_UNDERFLOW
    return 0;
  f64 f = 0;
  // TODO: Make this machine independent (encode IEEE754 floats
  // yourself?)
  memcpy(&f, vm->stack.data + vm->stack.pointer - FLOAT_SIZE, FLOAT_SIZE);
  vm->stack.pointer -= FLOAT_SIZE;
  return f;
}

int main(void)
{
  puts("Hello, world!");
  return 0;
}
