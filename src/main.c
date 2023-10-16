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

void write_bytes_to_file(darr_t *bytes, const char *filepath)
{
  FILE *fp    = fopen(filepath, "wb");
  size_t size = fwrite(bytes->data, bytes->used, 1, fp);
  fclose(fp);
  assert(size == 1);
}

void read_bytes_from_file(const char *filepath, darr_t *darr)
{
  darr->data      = NULL;
  darr->used      = 0;
  darr->available = 0;

  FILE *fp = fopen(filepath, "rb");
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  darr_init(darr, size);
  fseek(fp, 0, SEEK_SET);
  size_t read = fread(darr->data, size, 1, fp);
  fclose(fp);
  assert(read == 1);
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
