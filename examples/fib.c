/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the GPLv2
 * license.  You should have received a copy of the GPLv2 license with
 * this file.  If not, please write to: aryadev@aryadevchavali.com.

 * Created: 2023-10-23
 * Author: Aryadev Chavali
 * Description: An example virtual machine program which computes and
 * prints fibonacci numbers.  Note that by default the virtual machine
 * just rolls overflows over, so this program will never terminate.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <vm/inst.h>
#include <vm/runtime.h>

int main(void)
{
  inst_t instructions[] = {
      // MOV the values 1 and 1 to REG[0] and REG[1] respectively
      INST_PUSH(WORD, 1),
      INST_MOV(WORD, 0),
      INST_PUSH(WORD, 1),
      INST_MOV(WORD, 1),

      // Print value at register 0 with newline.
      INST_PUSH_REG(WORD, 0), // <-- #
      INST_PRINT(WORD),
      INST_PUSH(BYTE, '\n'),
      INST_PRINT(CHAR),

      // Print value at register 1 with newline
      INST_PUSH_REG(WORD, 1),
      INST_PRINT(WORD),
      INST_PUSH(BYTE, '\n'),
      INST_PRINT(CHAR),

      /* Compute the next pair of fibonacci numbers */
      // REG[0] + REG[1]
      INST_PUSH_REG(WORD, 0),
      INST_PUSH_REG(WORD, 1),
      INST_PLUS(WORD),

      // Mov REG[0] + REG[1] to REG[0]
      INST_MOV(WORD, 0),

      // REG[0] + REG[1]
      INST_PUSH_REG(WORD, 0),
      INST_PUSH_REG(WORD, 1),
      INST_PLUS(WORD),

      // Mov REG[0] + REG[1] to REG[1]
      INST_MOV(WORD, 1),

      // Jump to the point #
      INST_JUMP_ABS(4),
      INST_HALT,
  };

  byte stack[256];
  vm_t vm = {0};
  vm_load_stack(&vm, stack, ARR_SIZE(stack));
  vm_load_program(&vm, instructions, ARR_SIZE(instructions));
  err_t err = vm_execute_all(&vm);

  if (err)
  {
    const char *error_str = err_as_cstr(err);
    fprintf(stderr, "[ERROR]: %s\n", error_str);
    fprintf(stderr, "[ERROR]: VM Trace:\n");
    vm_print_all(&vm, stderr);
    return 255 - err;
  }
  return 0;
}
