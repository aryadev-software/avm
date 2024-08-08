/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Entrypoint to program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/base.h>
#include <lib/bytecode.h>
#include <lib/inst-macro.h>
#include <lib/inst.h>
#include <vm/runtime.h>
#include <vm/struct.h>

void usage(const char *program_name, FILE *out)
{
  fprintf(out,
          "Usage: %s [OPTIONS] FILE\n"
          "\t FILE: Bytecode file to execute\n"
          "\tOptions:\n"
          "\t\t To be developed...\n",
          program_name);
}

bool program_eq(prog_t *a, prog_t *b)
{
  if (memcmp(&a->header, &b->header, sizeof(a->header)))
  {
    printf("header not equivalent (a.count=%lu, a.start=%lu) and (b.count=%lu, "
           "b.start=%lu)\n",
           a->header.count, a->header.start, b->header.count, b->header.start);
    return false;
  }
  for (word_t i = 0; i < a->header.count; ++i)
  {
    if (!(a->instructions[i].opcode == b->instructions[i].opcode &&
          a->instructions[i].n == b->instructions[i].n))
    {
      printf("[%lu]: Not equivalent:\n\t", i);
      inst_print(stdout, a->instructions[i]);
      printf("\n\t");
      inst_print(stdout, b->instructions[i]);
      return false;
    }
    else if (IS_OPCODE_BINARY(a->instructions[i].opcode) &&
             memcmp(a->instructions[i].operands, b->instructions[i].operands,
                    WORD_SIZE))
    {
      printf("[%lu]: Not equivalent:\n\t", i);
      inst_print(stdout, a->instructions[i]);
      printf("\n\t");
      inst_print(stdout, b->instructions[i]);
      return false;
    }
    else if (IS_OPCODE_NARY(a->instructions[i].opcode) &&
             memcmp(a->instructions[i].operands, b->instructions[i].operands,
                    a->instructions[i].n))
    {
      printf("[%lu]: Not equivalent:\n\t", i);
      inst_print(stdout, a->instructions[i]);
      printf("\n\t");
      inst_print(stdout, b->instructions[i]);
      return false;
    }
  }
  return true;
}

int main(void)
{
#if 0
  size_t stack_size     = 256;
  byte_t *stack         = calloc(stack_size, 1);
  size_t registers_size = 8 * WORD_SIZE;
  byte_t *registers     = calloc(registers_size, 1);
  heap_t heap           = {0};
  heap_create(&heap);
  size_t call_stack_size = 256;
  word_t *call_stack     = calloc(call_stack_size, sizeof(call_stack));
#endif

  byte_t op_bytes[1024];
  op_bytes[0] = 0xDE;
  op_bytes[1] = 0xAD;
  op_bytes[2] = 0xBE;
  op_bytes[3] = 0xEF;
  op_bytes[4] = 0x0;

  // Write program here
  inst_t instructions[] = {
      INST_PUSH(4, op_bytes),
      INST_PUSH(4, op_bytes),
      INST_MOV(9, op_bytes + 4),
      INST_NOOP,
      INST_HALT,
  };

  prog_t program = {{0, ARR_SIZE(instructions)}, instructions};
  printf("program.start=%lu, program.count=%lu\nprogram={\n",
         program.header.start, program.header.count);
  for (size_t i = 0; i < program.header.count; ++i)
  {
    printf("\t%lu: ", i);
    inst_print(stdout, program.instructions[i]);
    printf("\n");
  }
  printf("}\n");

  bytecode_t writer;
  darr_init(&writer, bytecode_prog_size(program));
  bytecode_write_prog_header(&writer, program.header);
  for (size_t i = 0; i < program.header.count; ++i)
  {
    bytecode_write_inst(&writer, program.instructions[i]);
  }

  bytecode_t reader;
  darr_init(&reader, writer.used);
  memcpy(reader.data, writer.data, writer.used);
  reader.used      = 0;
  reader.available = writer.used;
  for (word_t i = 0; i < reader.available; ++i)
  {
    printf("\t[%lu]: %x\n", i, reader.data[i]);
  }
  prog_t read_program = {0};
  bool success = bytecode_read_prog_header(&reader, &read_program.header);
  if (success)
  {
    printf("read_program.start=%lu, read_program.count=%lu\nread_program={\n",
           read_program.header.start, read_program.header.count);
    read_program.instructions =
        calloc(program.header.count, sizeof(*program.instructions));
    for (size_t i = 0; i < program.header.count; ++i)
    {
      printf("\tread[%lu/%lu/", i, reader.used);
      bool s = bytecode_read_inst(&reader, read_program.instructions + i);
      if (!s)
      {
        printf("Reading %lu (%lu) not successful\n", i, reader.used);
        exit(1);
      }
      printf("%lu]: ", reader.used);
      inst_print(stdout, read_program.instructions[i]);
      printf("\n");
    }
    printf("}\nread_program_summary={\n");
    for (size_t i = 0; i < read_program.header.count; ++i)
    {
      printf("\t%lu: ", i);
      inst_print(stdout, read_program.instructions[i]);
      printf("\n");
    }
    printf("}");
    bool b = program_eq(&program, &read_program);
    if (!b)
    {
      printf("Read != Write\n");
      exit(1);
    }
    free(read_program.instructions);
  }
  else
  {
    printf("Not successful");
  }

  free(writer.data);
  free(reader.data);

#if 0
  vm_t vm = {0};
  vm_load_stack(&vm, stack, stack_size);
  vm_load_program(&vm, program);
  vm_load_registers(&vm, registers, registers_size);
  vm_load_heap(&vm, heap);
  vm_load_call_stack(&vm, call_stack, call_stack_size);

  err_t err = vm_execute_all(&vm);
  if (err)
  {
    const char *error_str = err_as_cstr(err);
    FAIL("ERROR", "%s\n", error_str);
    vm_print_all(&vm, stderr);
  }
  vm_stop(&vm);

  free(stack);
  free(registers);
  free(call_stack);
#endif
  return 0;
}

#if 0
int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    usage(argv[0], stderr);
    return 1;
  }
  const char *filename = argv[1];

#if VERBOSE >= 1
  INFO("INTERPRETER", "`%s`\n", filename);
#endif

  FILE *fp        = fopen(filename, "rb");
  darr_t fp_bytes = darr_read_file(fp);
  fclose(fp);

  prog_t program = {0};
  size_t header_read =
      prog_read_header(&program, fp_bytes.data, fp_bytes.available);

  if (!header_read)
  {
    FAIL("ERROR", "Could not deserialise program header in `%s`\n", filename);
    return 1;
  }
  // Ensure that we MUST have something to read
  else if (program.count == 0)
    return 0;

  // After reading header, we can allocate the buffer of instrutions exactly
  program.instructions = calloc(program.count, sizeof(*program.instructions));
  size_t bytes_read    = 0;
  read_err_prog_t read_err =
      prog_read_instructions(&program, &bytes_read, fp_bytes.data + header_read,
                             fp_bytes.available - header_read);

  if (bytes_read == 0)
  {
    FAIL("ERROR", "%s [%lu]:", filename, read_err.index);
    switch (read_err.type)
    {
    case READ_ERR_INVALID_OPCODE:
      fprintf(stderr, "INVALID_OPCODE");
      break;
    case READ_ERR_OPERAND_NO_FIT:
      fprintf(stderr, "OPERAND_NO_FIT");
      break;
    case READ_ERR_EXPECTED_MORE:
      fprintf(stderr, "EXPECTED_MORE");
      break;
    case READ_ERR_END:
    default:
      fprintf(stderr, "UNKNOWN");
      break;
    }
    fprintf(stderr, "\n");
    return 1;
  }

#if VERBOSE >= 1
  SUCCESS("SETUP", "Read %lu instructions\n", program.count);
#endif

  size_t stack_size     = 256;
  byte_t *stack         = calloc(stack_size, 1);
  size_t registers_size = 8 * WORD_SIZE;
  byte_t *registers     = calloc(registers_size, 1);
  heap_t heap           = {0};
  heap_create(&heap);
  size_t call_stack_size = 256;
  word_t *call_stack     = calloc(call_stack_size, sizeof(call_stack));

  vm_t vm = {0};
  vm_load_stack(&vm, stack, stack_size);
  vm_load_program(&vm, program);
  vm_load_registers(&vm, registers, registers_size);
  vm_load_heap(&vm, heap);
  vm_load_call_stack(&vm, call_stack, call_stack_size);

#if VERBOSE >= 1
  SUCCESS("SETUP", "Loaded internals\n%s", "");
  INFO("INTERPRETER", "Beginning execution\n%s", "");
#endif
  err_t err = vm_execute_all(&vm);

  int ret = 0;
  if (err)
  {
    const char *error_str = err_as_cstr(err);
    FAIL("ERROR", "%s\n", error_str);
    vm_print_all(&vm, stderr);
    ret = 255 - err;
  }

  vm_stop(&vm);

#if VERBOSE >= 1
  SUCCESS("INTEPRETER", "Finished execution\n%s", "");
#endif
  return ret;
}
#endif
