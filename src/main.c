/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Entrypoint to program
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "./inst.h"
#include "./runtime.h"

int interpret_bytecode(const char *filepath)
{
  FILE *fp             = fopen(filepath, "rb");
  size_t number        = 0;
  inst_t *instructions = insts_read_bytecode_file(fp, &number);
  fclose(fp);

  byte stack[256];
  vm_t vm = {0};
  vm_load_stack(&vm, stack, ARR_SIZE(stack));
  vm_load_program(&vm, instructions, number);
  for (size_t i = 0; i < number; ++i)
  {
    vm_execute(&vm);
    printf("Cycle %lu\n", i);
    vm_print_all(&vm, stdout);
    printf("\n");
  }
  free(instructions);
  return 0;
}

int assemble_instructions(inst_t *instructions, size_t number,
                          const char *filepath)
{
  FILE *fp = fopen(filepath, "wb");
  insts_write_bytecode_file(instructions, number, fp);
  fclose(fp);
  return 0;
}

int main(void)
{
  byte stack_data[256];
  vm_t vm = {0};
  vm_load_stack(&vm, stack_data, ARR_SIZE(stack_data));
  inst_t instructions[] = {
      INST_BPUSH(0xfa),  INST_BMOV(0),      INST_BPUSH(0xfb),
      INST_BMOV(1),      INST_BPUSH(0xfc),  INST_BMOV(2),
      INST_BPUSH(0xfd),  INST_BMOV(3),      INST_BPUSH_REG(3),
      INST_BPUSH_REG(2), INST_BPUSH_REG(1), INST_BPUSH_REG(0),
  };
  vm_load_program(&vm, instructions, ARR_SIZE(instructions));
  for (size_t i = 0; i < ARR_SIZE(instructions); ++i)
    vm_execute(&vm);
  return 0;
}
