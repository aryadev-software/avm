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

#include "./inst.h"
#include "./runtime.h"

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
