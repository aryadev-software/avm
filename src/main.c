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

typedef struct
{
  struct Stack
  {
    byte *data;
    word ptr, size;
  } stack;
} vm_t;

void vm_load_stack(vm_t *vm, byte *bytes, size_t size)
{
  vm->stack.data = bytes;
  vm->stack.size = size;
  vm->stack.ptr  = 0;
}

void vm_push_byte(vm_t *vm, data_t b)
{
  if (vm->stack.ptr >= vm->stack.size)
    // TODO: Error STACK_OVERFLOW
    return;
  vm->stack.data[vm->stack.ptr++] = b.as_byte;
}

void vm_push_word(vm_t *vm, data_t w)
{
  if (vm->stack.ptr + WORD_SIZE >= vm->stack.size)
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
  if (vm->stack.ptr + FLOAT_SIZE >= vm->stack.size)
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

int main(void)
{
  puts("Hello, world!");
  return 0;
}
