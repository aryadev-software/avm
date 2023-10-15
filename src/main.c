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

#define VM_STACK_MAX 1024

typedef struct
{
  struct Stack
  {
    byte data[VM_STACK_MAX];
    word pointer;
  } stack;
} vm_t;

void vm_push_byte(vm_t *vm, byte b)
{
  if (vm->stack.pointer >= VM_STACK_MAX)
    return;
  vm->stack.data[vm->stack.pointer++] = b;
}

void vm_push_word(vm_t *vm, word w)
{
  // NOTE: Relies on sizeof measuring in bytes
  if (vm->stack.pointer + sizeof(w) >= VM_STACK_MAX)
    return;
  memcpy(vm->stack.data + vm->stack.pointer, &w, sizeof(w));
  vm->stack.pointer += sizeof(w);
}

int main(void)
{
  puts("Hello, world!");
  return 0;
}
