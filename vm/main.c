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

#include <lib/inst.h>

#include "./runtime.h"
#include "./struct.h"

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
  printf("[" TERM_YELLOW "INTERPRETER" TERM_RESET "]: `%s`\n", filename);
#endif

  FILE *fp        = fopen(filename, "rb");
  prog_t *program = prog_read_file(fp);
  fclose(fp);

#if VERBOSE >= 1
  printf("\t[" TERM_GREEN "SETUP" TERM_RESET "]: Read %lu instructions\n",
         program->count);
#endif

  size_t stack_size     = 256;
  byte_t *stack         = calloc(stack_size, 1);
  registers_t registers = {0};
  darr_init(&registers, 8 * WORD_SIZE);
  heap_t heap = {0};
  heap_create(&heap);
  size_t call_stack_size = 256;
  word_t *call_stack     = calloc(call_stack_size, sizeof(call_stack));

  vm_t vm = {0};
  vm_load_stack(&vm, stack, stack_size);
  vm_load_program(&vm, program);
  vm_load_registers(&vm, registers);
  vm_load_heap(&vm, heap);
  vm_load_call_stack(&vm, call_stack, call_stack_size);

#if VERBOSE >= 1
  printf("\t[" TERM_GREEN "SETUP" TERM_RESET
         "]: Loaded stack and program into VM\n");
#endif
#if VERBOSE >= 1
  printf("[" TERM_YELLOW "INTERPRETER" TERM_RESET "]: Beginning execution\n");
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

  vm_stop(&vm);

#if VERBOSE >= 1
  printf("[%sINTERPRETER%s]: Finished execution\n", TERM_GREEN, TERM_RESET);
#endif
  return ret;
}
