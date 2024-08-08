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
  case ERR_INVALID_REGISTER_BYTE:
    return "INVALID_REGISTER_BYTE";
  case ERR_INVALID_REGISTER_SHORT:
    return "INVALID_REGISTER_SHORT";
  case ERR_INVALID_REGISTER_HWORD:
    return "INVALID_REGISTER_HWORD";
  case ERR_INVALID_REGISTER_WORD:
    return "INVALID_REGISTER_WORD";
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
  const size_t count      = program->data.count;
  err_t err               = ERR_OK;
  // Setup the initial address according to the program
  program->ptr = program->data.start_address;
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
  if (w >= vm->program.data.count)
    return ERR_INVALID_PROGRAM_ADDRESS;
  vm->program.ptr = w;
  return ERR_OK;
}

err_t vm_push_byte(vm_t *vm, data_t b)
{
  if (vm->stack.ptr >= vm->stack.max)
    return ERR_STACK_OVERFLOW;
  vm->stack.data[vm->stack.ptr++] = b.as_byte;
  return ERR_OK;
}

/* Pushing value onto the stack

   Convert the datum into LE ordering then push onto the stack byte by
   byte.  This means that the MSB of any datum is at the top of the
   stack by the end of the procedure.  If we look at the stack from
   top down the number is in big endian, but from bottom up the number
   is in little endian.

   This is the same as just converting some datum into LE ordered
   bytes, which convert_<TYPE>_to_bytes does.

   Consider halfword 0x89ABCDEF.
   1) Bytes are {0xEF, 0xCD, 0xAB, 0x89}
   2) Stack top is 0x89 */
#define VM_PUSH_CONSTR(TYPE, TYPE_CAP)                                      \
  err_t vm_push_##TYPE(vm_t *vm, data_t f)                                  \
  {                                                                         \
    if (vm->stack.ptr + TYPE_CAP##_SIZE >= vm->stack.max)                   \
      return ERR_STACK_OVERFLOW;                                            \
    convert_##TYPE##_to_bytes(f.as_##TYPE, vm->stack.data + vm->stack.ptr); \
    vm->stack.ptr += TYPE_CAP##_SIZE;                                       \
    return ERR_OK;                                                          \
  }

VM_PUSH_CONSTR(short, SHORT)
VM_PUSH_CONSTR(hword, HWORD)
VM_PUSH_CONSTR(word, WORD)

err_t vm_pop_byte(vm_t *vm, data_t *ret)
{
  if (vm->stack.ptr == 0)
    return ERR_STACK_UNDERFLOW;
  *ret = DBYTE(vm->stack.data[--vm->stack.ptr]);
  return ERR_OK;
}

/* Popping a value from the stack and storing it

   Since data is pushed in little endian format onto the stack, such
   that the MSB is the top of the stack, copying N bytes from (top -
   N) to N gives us the exact bytes of the datum in little endian.

   Convert that into host order and we're done.

   Consider halfword 0x89ABCDEF.  When pushed onto the stack, looking
   at the stack from bottom up there are the values {0xEF, 0xCD, 0xAB,
   0x89} where 0x89 is at the top of the stack.

   1) Bytes are copied into buffer {0xEF, 0xCD, 0xAB, 0x89}
   2) Value is converted into host order datum */

#define VM_POP_CONSTR(TYPE, TYPE_CAP)                                 \
  err_t vm_pop_##TYPE(vm_t *vm, data_t *ret)                          \
  {                                                                   \
    if (vm->stack.ptr < TYPE_CAP##_SIZE)                              \
      return ERR_STACK_UNDERFLOW;                                     \
    byte_t bytes[TYPE_CAP##_SIZE] = {0};                              \
    memcpy(bytes, vm->stack.data + (vm->stack.ptr - TYPE_CAP##_SIZE), \
           TYPE_CAP##_SIZE);                                          \
    *ret = D##TYPE_CAP(convert_bytes_to_##TYPE(bytes));               \
    return ERR_OK;                                                    \
  }

VM_POP_CONSTR(short, SHORT)
VM_POP_CONSTR(hword, HWORD)
VM_POP_CONSTR(word, WORD)

/* Pushing the value stored at a register onto the stack.

   MOV stores any set of bytes on the stack (which is a datum in
   Little Endian) in the same order in the register i.e. a datum X
   will be ordered the exact same way on the stack as in a register.

   So instead of getting the value from a register, converting it to
   host order, then pushing the datum (which converts it back to
   little endian), let's just memcpy the value from the register to
   the stack.

   Note this means that we check for stack overflow here.
 */
#define VM_PUSH_REGISTER_CONSTR(TYPE, TYPE_CAP)                             \
  err_t vm_push_##TYPE##_register(vm_t *vm, word_t reg)                     \
  {                                                                         \
    if (reg > (vm->registers.size / TYPE_CAP##_SIZE))                       \
      return ERR_INVALID_REGISTER_##TYPE_CAP;                               \
    else if (vm->stack.ptr + TYPE_CAP##_SIZE >= vm->stack.max)              \
      return ERR_STACK_OVERFLOW;                                            \
    memcpy(vm->stack.data + vm->stack.ptr,                                  \
           vm->registers.bytes + (reg * TYPE_CAP##_SIZE), TYPE_CAP##_SIZE); \
    vm->stack.ptr += TYPE_CAP##_SIZE;                                       \
    return ERR_OK;                                                          \
  }

VM_PUSH_REGISTER_CONSTR(byte, BYTE)
VM_PUSH_REGISTER_CONSTR(short, SHORT)
VM_PUSH_REGISTER_CONSTR(hword, HWORD)
VM_PUSH_REGISTER_CONSTR(word, WORD)

/* Move a value from the stack into a specific register.

   Values are stored in LE order on the stack.  Values in registers
   should be in LE order as well for consistency.  Which means if, for
   a value of N bytes, the array stack[top - N:top] is copied into the
   register directly, we're done.
*/
#define VM_MOV_CONSTR(TYPE, TYPE_CAP)                          \
  err_t vm_mov_##TYPE(vm_t *vm, word_t reg)                    \
  {                                                            \
    if (reg >= (vm->registers.size / TYPE_CAP##_SIZE))         \
      return ERR_INVALID_REGISTER_##TYPE_CAP;                  \
    else if (vm->stack.ptr + TYPE_CAP##_SIZE >= vm->stack.max) \
      return ERR_STACK_OVERFLOW;                               \
    memcpy(vm->registers.bytes + (reg * TYPE_CAP##_SIZE),      \
           vm->stack.data + vm->stack.ptr - (TYPE_CAP##_SIZE), \
           TYPE_CAP##_SIZE);                                   \
    vm->stack.ptr -= TYPE_CAP##_SIZE;                          \
    return ERR_OK;                                             \
  }

VM_MOV_CONSTR(byte, BYTE)
VM_MOV_CONSTR(short, SHORT)
VM_MOV_CONSTR(hword, HWORD)
VM_MOV_CONSTR(word, WORD)

err_t vm_dup_byte(vm_t *vm, word_t w)
{
  if (vm->stack.ptr < w + 1)
    return ERR_STACK_UNDERFLOW;
  return vm_push_byte(vm, DBYTE(vm->stack.data[vm->stack.ptr - 1 - w]));
}

#define VM_DUP_CONSTR(TYPE, TYPE_CAP)                                    \
  err_t vm_dup_##TYPE(vm_t *vm, word_t w)                                \
  {                                                                      \
    if (vm->stack.ptr < TYPE_CAP##_SIZE * (w + 1))                       \
      return ERR_STACK_UNDERFLOW;                                        \
    else if (vm->stack.ptr + TYPE_CAP##_SIZE >= vm->stack.max)           \
      return ERR_STACK_OVERFLOW;                                         \
    memcpy(vm->stack.data + vm->stack.ptr,                               \
           vm->stack.data + vm->stack.ptr - (TYPE_CAP##_SIZE * (w + 1)), \
           TYPE_CAP##_SIZE);                                             \
    vm->stack.ptr += TYPE_CAP##_SIZE;                                    \
    return ERR_OK;                                                       \
  }

VM_DUP_CONSTR(short, SHORT)
VM_DUP_CONSTR(hword, HWORD)
VM_DUP_CONSTR(word, WORD)

#define VM_MALLOC_CONSTR(TYPE, TYPE_CAP)                                  \
  err_t vm_malloc_##TYPE(vm_t *vm)                                        \
  {                                                                       \
    data_t n  = {0};                                                      \
    err_t err = vm_pop_word(vm, &n);                                      \
    if (err)                                                              \
      return err;                                                         \
    page_t *page = heap_allocate(&vm->heap, n.as_word * TYPE_CAP##_SIZE); \
    return vm_push_word(vm, DWORD((word_t)page));                         \
  }

VM_MALLOC_CONSTR(byte, BYTE)
VM_MALLOC_CONSTR(short, SHORT)
VM_MALLOC_CONSTR(hword, HWORD)
VM_MALLOC_CONSTR(word, WORD)

#define VM_MSET_CONSTR(TYPE, TYPE_CAP)                           \
  err_t vm_mset_##TYPE(vm_t *vm)                                 \
  {                                                              \
    data_t n  = {0};                                             \
    err_t err = vm_pop_word(vm, &n);                             \
    if (err)                                                     \
      return err;                                                \
    data_t object = {0};                                         \
    err           = vm_pop_##TYPE(vm, &object);                  \
    if (err)                                                     \
      return err;                                                \
    data_t ptr = {0};                                            \
    err        = vm_pop_word(vm, &ptr);                          \
    if (err)                                                     \
      return err;                                                \
    page_t *page = (page_t *)ptr.as_word;                        \
    if (n.as_word >= (page->available / TYPE_CAP##_SIZE))        \
      return ERR_OUT_OF_BOUNDS;                                  \
    DARR_AT(TYPE##_t, page->data, n.as_word) = object.as_##TYPE; \
    return ERR_OK;                                               \
  }

VM_MSET_CONSTR(byte, BYTE)
VM_MSET_CONSTR(short, SHORT)
VM_MSET_CONSTR(hword, HWORD)
VM_MSET_CONSTR(word, WORD)

#define VM_MGET_CONSTR(TYPE, TYPE_CAP)                                     \
  err_t vm_mget_##TYPE(vm_t *vm)                                           \
  {                                                                        \
    data_t n  = {0};                                                       \
    err_t err = vm_pop_word(vm, &n);                                       \
    if (err)                                                               \
      return (err);                                                        \
    data_t ptr = {0};                                                      \
    err        = vm_pop_word(vm, &ptr);                                    \
    if (err)                                                               \
      return err;                                                          \
    page_t *page = (page_t *)ptr.as_word;                                  \
    if (n.as_word >= (page->available / TYPE_CAP##_SIZE))                  \
      return ERR_OUT_OF_BOUNDS;                                            \
    else if (vm->stack.ptr + TYPE_CAP##_SIZE >= vm->stack.max)             \
      return ERR_STACK_OVERFLOW;                                           \
    memcpy(vm->stack.data + vm->stack.ptr,                                 \
           page->data + (n.as_word * (TYPE_CAP##_SIZE)), TYPE_CAP##_SIZE); \
    vm->stack.ptr += TYPE_CAP##_SIZE;                                      \
    return ERR_OK;                                                         \
  }

VM_MGET_CONSTR(byte, BYTE)
VM_MGET_CONSTR(short, SHORT)
VM_MGET_CONSTR(hword, HWORD)
VM_MGET_CONSTR(word, WORD)

err_t vm_mdelete(vm_t *vm)
{
  data_t ptr = {0};
  err_t err  = vm_pop_word(vm, &ptr);
  if (err)
    return err;
  page_t *page = (page_t *)ptr.as_word;
  bool done    = heap_free(&vm->heap, page);
  if (!done)
    return ERR_INVALID_PAGE_ADDRESS;
  return ERR_OK;
}

err_t vm_msize(vm_t *vm)
{
  data_t ptr = {0};
  err_t err  = vm_pop_word(vm, &ptr);
  if (err)
    return err;
  page_t *page = (page_t *)ptr.as_word;
  return vm_push_word(vm, DWORD(page->available));
}

// TODO: rename this to something more appropriate
#define VM_NOT_TYPE(TYPEL, TYPEU)                        \
  err_t vm_not_##TYPEL(vm_t *vm)                         \
  {                                                      \
    data_t a  = {0};                                     \
    err_t err = vm_pop_##TYPEL(vm, &a);                  \
    if (err)                                             \
      return err;                                        \
    return vm_push_##TYPEL(vm, D##TYPEU(!a.as_##TYPEL)); \
  }

VM_NOT_TYPE(byte, BYTE)
VM_NOT_TYPE(short, SHORT)
VM_NOT_TYPE(hword, HWORD)
VM_NOT_TYPE(word, WORD)

// TODO: rename this to something more appropriate
#define VM_SAME_TYPE(COMPNAME, COMP, TYPEL, TYPEU)                        \
  err_t vm_##COMPNAME##_##TYPEL(vm_t *vm)                                 \
  {                                                                       \
    data_t a = {0}, b = {0};                                              \
    err_t err = vm_pop_##TYPEL(vm, &a);                                   \
    if (err)                                                              \
      return err;                                                         \
    err = vm_pop_##TYPEL(vm, &b);                                         \
    if (err)                                                              \
      return err;                                                         \
    return vm_push_##TYPEL(vm, D##TYPEU(a.as_##TYPEL COMP b.as_##TYPEL)); \
  }

// TODO: rename this to something more appropriate
#define VM_COMPARATOR_TYPE(COMPNAME, COMP, TYPEL, GETL)           \
  err_t vm_##COMPNAME##_##GETL(vm_t *vm)                          \
  {                                                               \
    data_t a = {0}, b = {0};                                      \
    err_t err = vm_pop_##TYPEL(vm, &a);                           \
    if (err)                                                      \
      return err;                                                 \
    err = vm_pop_##TYPEL(vm, &b);                                 \
    if (err)                                                      \
      return err;                                                 \
    return vm_push_byte(vm, DBYTE(b.as_##GETL COMP a.as_##GETL)); \
  }

VM_SAME_TYPE(or, |, byte, BYTE)
VM_SAME_TYPE(or, |, short, SHORT)
VM_SAME_TYPE(or, |, hword, HWORD)
VM_SAME_TYPE(or, |, word, WORD)

VM_SAME_TYPE(and, &, byte, BYTE)
VM_SAME_TYPE(and, &, short, SHORT)
VM_SAME_TYPE(and, &, hword, HWORD)
VM_SAME_TYPE(and, &, word, WORD)

VM_SAME_TYPE(xor, ^, byte, BYTE)
VM_SAME_TYPE(xor, ^, short, SHORT)
VM_SAME_TYPE(xor, ^, hword, HWORD)
VM_SAME_TYPE(xor, ^, word, WORD)

VM_SAME_TYPE(plus, +, byte, BYTE)
VM_SAME_TYPE(plus, +, short, SHORT)
VM_SAME_TYPE(plus, +, hword, HWORD)
VM_SAME_TYPE(plus, +, word, WORD)

VM_SAME_TYPE(sub, -, byte, BYTE)
VM_SAME_TYPE(sub, -, short, SHORT)
VM_SAME_TYPE(sub, -, hword, HWORD)
VM_SAME_TYPE(sub, -, word, WORD)

VM_SAME_TYPE(mult, *, byte, BYTE)
VM_SAME_TYPE(mult, *, short, SHORT)
VM_SAME_TYPE(mult, *, hword, HWORD)
VM_SAME_TYPE(mult, *, word, WORD)

VM_COMPARATOR_TYPE(eq, ==, byte, byte)
VM_COMPARATOR_TYPE(eq, ==, byte, sbyte)
VM_COMPARATOR_TYPE(eq, ==, short, short)
VM_COMPARATOR_TYPE(eq, ==, short, sshort)
VM_COMPARATOR_TYPE(eq, ==, hword, hword)
VM_COMPARATOR_TYPE(eq, ==, hword, shword)
VM_COMPARATOR_TYPE(eq, ==, word, word)
VM_COMPARATOR_TYPE(eq, ==, word, sword)

VM_COMPARATOR_TYPE(lt, <, byte, byte)
VM_COMPARATOR_TYPE(lt, <, byte, sbyte)
VM_COMPARATOR_TYPE(lt, <, short, short)
VM_COMPARATOR_TYPE(lt, <, short, sshort)
VM_COMPARATOR_TYPE(lt, <, hword, hword)
VM_COMPARATOR_TYPE(lt, <, hword, shword)
VM_COMPARATOR_TYPE(lt, <, word, word)
VM_COMPARATOR_TYPE(lt, <, word, sword)

VM_COMPARATOR_TYPE(lte, <=, byte, byte)
VM_COMPARATOR_TYPE(lte, <=, byte, sbyte)
VM_COMPARATOR_TYPE(lte, <=, short, short)
VM_COMPARATOR_TYPE(lte, <=, short, sshort)
VM_COMPARATOR_TYPE(lte, <=, hword, hword)
VM_COMPARATOR_TYPE(lte, <=, hword, shword)
VM_COMPARATOR_TYPE(lte, <=, word, word)
VM_COMPARATOR_TYPE(lte, <=, word, sword)

VM_COMPARATOR_TYPE(gt, >, byte, byte)
VM_COMPARATOR_TYPE(gt, >, byte, sbyte)
VM_COMPARATOR_TYPE(gt, >, short, short)
VM_COMPARATOR_TYPE(gt, >, short, sshort)
VM_COMPARATOR_TYPE(gt, >, hword, hword)
VM_COMPARATOR_TYPE(gt, >, hword, shword)
VM_COMPARATOR_TYPE(gt, >, word, word)
VM_COMPARATOR_TYPE(gt, >, word, sword)

VM_COMPARATOR_TYPE(gte, >=, byte, byte)
VM_COMPARATOR_TYPE(gte, >=, byte, sbyte)
VM_COMPARATOR_TYPE(gte, >=, short, short)
VM_COMPARATOR_TYPE(gte, >=, short, sshort)
VM_COMPARATOR_TYPE(gte, >=, hword, hword)
VM_COMPARATOR_TYPE(gte, >=, hword, shword)
VM_COMPARATOR_TYPE(gte, >=, word, word)
VM_COMPARATOR_TYPE(gte, >=, word, sword)
