/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Virtual machine implementation
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#if VERBOSE >= 2
#include <string.h>
#endif

#include <vm/runtime.h>

const char *err_as_cstr(err_t err)
{
  switch (err)
  {
  case ERR_OK:
    return "OK";
  case ERR_STACK_UNDERFLOW:
    return "STACK_UNDERFLOW";
  case ERR_STACK_OVERFLOW:
    return "STACK_OVERFLOW";
  case ERR_CALL_STACK_UNDERFLOW:
    return "CALL_STACK_UNDERFLOW";
  case ERR_CALL_STACK_OVERFLOW:
    return "CALL_STACK_OVERFLOW";
  case ERR_INVALID_OPCODE:
    return "INVALID_OPCODE";
  case ERR_INVALID_REGISTER:
    return "INVALID_REGISTER";
  case ERR_INVALID_REGISTER_FIT:
    return "INVALID_REGISTER_FIT";
  case ERR_INVALID_PROGRAM_ADDRESS:
    return "INVALID_PROGRAM_ADDRESS";
  case ERR_INVALID_PAGE_ADDRESS:
    return "INVALID_PAGE_ADDRESS";
  case ERR_OUT_OF_BOUNDS:
    return "OUT_OF_BOUNDS";
  case ERR_END_OF_PROGRAM:
    return "END_OF_PROGRAM";
  default:
    return "";
  }
}

static_assert(NUMBER_OF_OPCODES == 30, "vm_execute: Out of date");

err_t vm_execute(vm_t *vm)
{
  struct Program *prog = &vm->program;
  prog_t program_data  = prog->data;
  if (prog->ptr >= program_data.header.count)
    return ERR_END_OF_PROGRAM;
  inst_t instruction = program_data.instructions[prog->ptr];

  // Opcodes which defer to another function using lookup table
  if (!IS_OPCODE(instruction.opcode))
    return ERR_INVALID_OPCODE;
  else if (instruction.opcode == OP_PUSH)
  {
    err_t err = vm_push(vm, instruction.n, instruction.operands);
    if (err)
      return err;
    prog->ptr++;
  }
  else if (instruction.opcode == OP_POP)
  {
    err_t err = vm_pop(vm, instruction.n, NULL);
    if (err)
      return err;
    prog->ptr++;
  }
  else if (instruction.opcode == OP_PUSH_REGISTER)
  {
    word_t reg = 0;
    memcpy(&reg, instruction.operands, WORD_SIZE);
    err_t err = vm_push_register(vm, instruction.n, reg);
    if (err)
      return err;
    prog->ptr++;
  }
  else if (instruction.opcode == OP_MOV)
  {
    word_t reg = 0;
    memcpy(&reg, instruction.operands, WORD_SIZE);
    err_t err = vm_mov(vm, instruction.n, reg);
    if (err)
      return err;
    prog->ptr++;
  }
  // Opcodes defined in loop
  else if (instruction.opcode == OP_JUMP_ABS)
    return vm_jump(vm, instruction.n);
  else if (instruction.opcode == OP_JUMP_IF)
  {
    // Pop instruction.n bytes
    byte_t *ops = NULL;
    err_t err   = vm_pop(vm, instruction.n, &ops);
    if (err)
      return err;
    // TODO: Can we do this faster?
    bool is_nonzero = false;
    for (size_t i = 0; i < instruction.n; ++i)
    {
      if (ops[i])
      {
        is_nonzero = true;
        break;
      }
    }

    if (is_nonzero)
    {
      word_t addr = 0;
      memcpy(&addr, instruction.operands, WORD_SIZE);
      err = vm_jump(vm, addr);
      if (err)
        return err;
    }
    else
    {
      ++prog->ptr;
    }
  }
  else if (instruction.opcode == OP_CALL)
  {
    if (vm->call_stack.ptr >= vm->call_stack.max)
      return ERR_CALL_STACK_OVERFLOW;
    vm->call_stack.address_pointers[vm->call_stack.ptr++] = vm->program.ptr + 1;
    return vm_jump(vm, instruction.n);
  }
  else if (instruction.opcode == OP_RET)
  {
    if (vm->call_stack.ptr == 0)
      return ERR_CALL_STACK_UNDERFLOW;
    word_t addr = vm->call_stack.address_pointers[vm->call_stack.ptr - 1];
    err_t err   = vm_jump(vm, vm->call_stack.address_pointers[addr]);
    if (err)
      return err;

    --vm->call_stack.ptr;
  }
  else if (instruction.opcode == OP_HALT)
  {
    // Do nothing here.  Should be caught by callers of vm_execute
  }
  else if (IS_OPCODE_VM_UNARY(instruction.opcode))
  {
    err_t err = UNARY_WORD_ROUTINES[instruction.opcode](vm, instruction.n);
    if (err)
      return err;
    prog->ptr++;
  }
  return ERR_OK;
}

err_t vm_execute_all(vm_t *vm)
{
  struct Program *program = &vm->program;
  const size_t count      = program->data.header.count;
  err_t err               = ERR_OK;
  // Setup the initial address according to the program
  program->ptr = program->data.header.start;
#if VERBOSE >= 1
  size_t cycles = 0;
#endif
#if VERBOSE >= 2
  struct Registers prev_registers = vm->registers;
  size_t prev_sptr                = 0;
  size_t prev_pages               = 0;
  size_t prev_cptr                = 0;
#endif
  while (program->ptr < count &&
         program->data.instructions[program->ptr].opcode != OP_HALT)
  {
#if VERBOSE >= 2
    INFO("vm_execute_all", "Trace(Cycle%lu)\n", cycles);
    fputs(
        "----------------------------------------------------------------------"
        "----------\n",
        stdout);
    vm_print_program(vm, stdout);
    fputs(
        "----------------------------------------------------------------------"
        "----------\n",
        stdout);
    if (prev_cptr != vm->call_stack.ptr)
    {
      vm_print_call_stack(vm, stdout);
      prev_cptr = vm->call_stack.ptr;
      fputs("------------------------------------------------------------------"
            "----"
            "----------\n",
            stdout);
    }
    if (prev_pages != HEAP_SIZE(vm->heap))
    {
      vm_print_heap(vm, stdout);
      prev_pages = HEAP_SIZE(vm->heap);
      fputs("------------------------------------------------------------------"
            "----"
            "----------\n",
            stdout);
    }
    if (memcmp(&prev_registers, &vm->registers, sizeof(vm->registers)) != 0)
    {
      vm_print_registers(vm, stdout);
      prev_registers = vm->registers;
      fputs("------------------------------------------------------------------"
            "----"
            "----------\n",
            stdout);
    }
    if (prev_sptr != vm->stack.ptr)
    {
      vm_print_stack(vm, stdout);
      prev_sptr = vm->stack.ptr;
      fputs("------------------------------------------------------------------"
            "----"
            "----------\n",
            stdout);
    }
#endif
#if VERBOSE >= 1
    ++cycles;
#endif
    err = vm_execute(vm);
    if (err)
      return err;
  }

#if VERBOSE >= 1
  INFO("vm_execute_all", "Final VM State(Cycle %lu)\n", cycles);
  vm_print_all(vm, stdout);
#endif
  return err;
}

err_t vm_jump(vm_t *vm, word_t w)
{
  if (w >= vm->program.data.header.count)
    return ERR_INVALID_PROGRAM_ADDRESS;
  vm->program.ptr = w;
  return ERR_OK;
}
