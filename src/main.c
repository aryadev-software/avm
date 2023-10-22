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
  vm_execute_all(&vm);
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

int main(int argc, char *argv[])
{
  const char *filename = "out.bin";
  if (argc >= 2)
    filename = argv[1];
  inst_t instructions[] = {INST_HALT};
  assemble_instructions(instructions, ARR_SIZE(instructions), filename);
  interpret_bytecode(filename);
  return 0;
}
