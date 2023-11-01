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

#include "./runtime.h"
#include <lib/inst.h>

void usage(const char *program_name, FILE *out)
{
  fprintf(out,
          "Usage: %s [OPTIONS] FILE\n"
          "\t FILE: Bytecode file to execute\n"
          "\tOptions:\n"
          "\t\t To be developed...\n",
          program_name);
}

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    usage(argv[0], stderr);
    return 1;
  }
  const char *filename = argv[1];

#if VERBOSE >= 1
  printf("[%sINTERPRETER%s]: Interpreting `%s`\n", TERM_YELLOW, TERM_RESET,
         filename);
#endif

  FILE *fp             = fopen(filename, "rb");
  size_t number        = 0;
  inst_t *instructions = insts_read_bytecode_file(fp, &number);
  fclose(fp);

#if VERBOSE >= 1
  printf("\t[%sBYTECODE-READER%s]: Read %lu instructions\n", TERM_GREEN,
         TERM_RESET, number);
#endif

  byte stack[256];
  vm_t vm = {0};
  vm_load_stack(&vm, stack, ARR_SIZE(stack));
  vm_load_program(&vm, instructions, number);

#if VERBOSE >= 1
  printf("\t[%sVM-SETUP%s]: Loaded stack and program into VM\n", TERM_GREEN,
         TERM_RESET);
#endif
  err_t err = vm_execute_all(&vm);

  int ret = 0;
  if (err)
  {
    const char *error_str = err_as_cstr(err);
    fprintf(stderr, "[ERROR]: %s\n", error_str);
    vm_print_all(&vm, stderr);
    ret = 255 - err;
  }
  free(instructions);

#if VERBOSE >= 1
  printf("[%sINTERPRETER%s]: Finished execution\n", TERM_GREEN, TERM_RESET);
#endif
  return ret;
}
