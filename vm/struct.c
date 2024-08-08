/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2024-04-25
 * Author: Aryadev Chavali
 * Description: Virtual machine data structures and some helpers
 */

#include <stdio.h>
#include <stdlib.h>

#include <lib/darr.h>
#include <vm/struct.h>

void vm_load_stack(vm_t *vm, byte_t *bytes, size_t size)
{
  vm->stack.data = bytes;
  vm->stack.max  = size;
  vm->stack.ptr  = 0;
}

void vm_load_program(vm_t *vm, prog_t program)
{
  vm->program.ptr  = 0;
  vm->program.data = program;
}

void vm_load_registers(vm_t *vm, byte_t *buffer, size_t size)
{
  vm->registers = (struct Registers){.size = size, .bytes = buffer};
}

void vm_load_heap(vm_t *vm, heap_t heap)
{
  vm->heap = heap;
}

void vm_load_call_stack(vm_t *vm, word_t *buffer, size_t size)
{
  vm->call_stack =
      (struct CallStack){.address_pointers = buffer, .ptr = 0, .max = size};
}

void vm_stop(vm_t *vm)
{
#if VERBOSE >= 1
  bool leaks = false;
  INFO("vm_stop", "Checking for leaks...\n%s", "");
  if (vm->call_stack.ptr > 0)
  {
    leaks = true;
    FAIL("vm_stop", "Call stack at %lu\n", vm->call_stack.ptr);
    FAIL("vm_stop", "Call stack trace:%s", "");
    for (size_t i = vm->call_stack.ptr; i > 0; --i)
    {
      word_t w = vm->call_stack.address_pointers[i - 1];
      printf("\t[%lu]: %lX", vm->call_stack.ptr - i, w);
      if (i != 1)
        printf(", ");
      printf("\n");
    }
  }
  if (HEAP_SIZE(vm->heap) > 0)
  {
    const size_t size_pages = HEAP_SIZE(vm->heap);
    leaks                   = true;
    size_t capacities[size_pages], total_capacity = 0;
    for (size_t i = 0; i < size_pages; ++i)
    {
      page_t *cur   = DARR_AT(page_t *, vm->heap.page_vec.data, i);
      capacities[i] = cur->available;
      total_capacity += capacities[i];
    }
    FAIL("vm_stop", "Heap: %luB (over %lu %s) not reclaimed\n", total_capacity,
         size_pages, size_pages == 1 ? "page" : "pages");
    for (size_t i = 0; i < size_pages; i++)
      printf("\t[%lu]: %luB lost\n", i, capacities[i]);
  }
  if (vm->stack.ptr > 0)
  {
    leaks = true;
    FAIL("vm_stop", "Stack: %luB not reclaimed\n", vm->stack.ptr);
  }
  if (leaks)
    FAIL("vm_stop", "Leaks found\n%s", "");
  else
    SUCCESS("vm_stop", "No leaks found\n%s", "");
#endif

  vm->registers = (struct Registers){0};
  vm->program   = (struct Program){0};
  vm->stack     = (struct Stack){0};
  vm->heap      = (heap_t){0};
}

void vm_print_registers(vm_t *vm, FILE *fp)
{
  struct Registers reg = vm->registers;
  fprintf(fp, "Registers.size = %luB/%luH/%luW\n", vm->registers.size,
          vm->registers.size / HWORD_SIZE, vm->registers.size / WORD_SIZE);
  fprintf(fp, "Registers.reg = [");
  for (size_t i = 0; i < (reg.size / WORD_SIZE); ++i)
  {
    fprintf(fp, "{%lu:%lX}", i, VM_NTH_REGISTER(reg, i));
    if (i != reg.size - 1)
      fprintf(fp, ", ");
  }
  fprintf(fp, "]\n");
}

void vm_print_stack(vm_t *vm, FILE *fp)
{
  struct Stack stack = vm->stack;
  fprintf(fp, "Stack.max  = %lu\nStack.ptr  = %lu\nStack.data = [", stack.max,
          stack.ptr);
  if (stack.ptr == 0)
  {
    fprintf(fp, "]\n");
    return;
  }
  printf("\n");
  for (size_t i = stack.ptr; i > 0; --i)
  {
    byte_t b = stack.data[i - 1];
    fprintf(fp, "\t%lu: %X", stack.ptr - i, b);
    if (i != 1)
      fprintf(fp, ", ");
    fprintf(fp, "\n");
  }
  fprintf(fp, "]\n");
}

void vm_print_program(vm_t *vm, FILE *fp)
{
  struct Program program = vm->program;
  const size_t count     = program.data.header.count;
  fprintf(fp,
          "Program.max          = %lu\nProgram.ptr          = "
          "%lu\nProgram.instructions = [\n",
          count, program.ptr);
  size_t beg = 0;
  if (program.ptr >= VM_PRINT_PROGRAM_EXCERPT)
  {
    fprintf(fp, "\t...\n");
    beg = program.ptr - VM_PRINT_PROGRAM_EXCERPT;
  }
  else
    beg = 0;
  size_t end = MIN(program.ptr + VM_PRINT_PROGRAM_EXCERPT, count);
  for (size_t i = beg; i < end; ++i)
  {
    fprintf(fp, "\t%lu: ", i);
    inst_print(fp, program.data.instructions[i]);
    if (i == program.ptr)
      fprintf(fp, " <---");
    fprintf(fp, "\n");
  }
  if (end != count)
    fprintf(fp, "\t...\n");
  fprintf(fp, "]\n");
}

void vm_print_heap(vm_t *vm, FILE *fp)
{
  heap_t heap             = vm->heap;
  const size_t heap_pages = heap.page_vec.used / sizeof(page_t *);
  fprintf(fp, "Heap.pages = %lu\nHeap.data = [", heap_pages);
  if (heap_pages == 0)
  {
    fprintf(fp, "]\n");
    return;
  }
  fprintf(fp, "\n");
  for (size_t i = 0; i < heap_pages; ++i)
  {
    page_t *cur = DARR_AT(page_t *, heap.page_vec.data, i);
    fprintf(fp, "\t[%lu]@%p: ", i, (void *)cur);
    if (!cur)
      fprintf(fp, "<NIL>\n");
    else
    {
      fprintf(fp, "{");
      for (size_t j = 0; j < cur->available; ++j)
      {
        if ((j % 8) == 0)
          fprintf(fp, "\n\t\t");
        fprintf(fp, "%x", cur->data[j]);
        if (j != cur->available - 1)
          fprintf(fp, ",\t");
      }
      fprintf(fp, "\n\t}\n");
    }
  }
  fprintf(fp, "]\n");
}

void vm_print_call_stack(vm_t *vm, FILE *fp)
{
  struct CallStack cs = vm->call_stack;
  fprintf(fp, "CallStack.max  = %lu\nCallStack.ptr  = %lu\nCallStack.data = [",
          cs.max, cs.ptr);
  if (cs.ptr == 0)
  {
    fprintf(fp, "]\n");
    return;
  }
  printf("\n");
  for (size_t i = cs.ptr; i > 0; --i)
  {
    word_t w = cs.address_pointers[i - 1];
    fprintf(fp, "\t%lu: %lX", cs.ptr - i, w);
    if (i != 1)
      fprintf(fp, ", ");
    fprintf(fp, "\n");
  }
  fprintf(fp, "]\n");
}

void vm_print_all(vm_t *vm, FILE *fp)
{
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
  vm_print_program(vm, fp);
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
  vm_print_call_stack(vm, fp);
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
  vm_print_heap(vm, fp);
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
  vm_print_registers(vm, fp);
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
  vm_print_stack(vm, fp);
  fputs("----------------------------------------------------------------------"
        "----------\n",
        fp);
}
