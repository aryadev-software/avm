/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

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

static_assert(NUMBER_OF_OPCODES == 129, "vm_execute: Out of date");

err_t vm_execute(vm_t *vm)
{
  struct Program *prog = &vm->program;
  prog_t program_data  = prog->data;
  if (prog->ptr >= program_data.count)
    return ERR_END_OF_PROGRAM;
  inst_t instruction = program_data.instructions[prog->ptr];

  // Opcodes which defer to another function using lookup table
  if (UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_PUSH))
  {
    err_t err = PUSH_ROUTINES[instruction.opcode](vm, instruction.operand);
    if (err)
      return err;
    prog->ptr++;
  }
  else if (UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_MOV) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_PUSH_REGISTER) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_DUP) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_MALLOC) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_MSET) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_MGET))
  {
    err_t err =
        WORD_ROUTINES[instruction.opcode](vm, instruction.operand.as_word);
    if (err)
      return err;
    prog->ptr++;
  }
  else if (UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_POP))
  {
    static_assert(DATA_TYPE_NIL == -1 && DATA_TYPE_WORD == 3,
                  "Code using OPCODE_DATA_TYPE for quick same type opcode "
                  "conversion may be out of date.");

    // NOTE: We always use the first register to hold the result of
    // this pop.

    // Here we add OP_MOV_BYTE and the data_type_t of the opcode to
    // get the right typed OP_MOV opcode.
    opcode_t mov_opcode =
        OPCODE_DATA_TYPE(instruction.opcode, OP_POP) + OP_MOV_BYTE;

    err_t err = WORD_ROUTINES[mov_opcode](vm, 0);
    if (err)
      return err;
    prog->ptr++;
  }
  else if (UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_NOT) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_OR) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_AND) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_XOR) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_EQ) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_PLUS) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_SUB) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_MULT) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_MALLOC_STACK) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_MSET_STACK) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_MGET_STACK) ||
           SIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_LT) ||
           SIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_LTE) ||
           SIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_GT) ||
           SIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_GTE) ||
           instruction.opcode == OP_MDELETE || instruction.opcode == OP_MSIZE)
  {
    err_t err = STACK_ROUTINES[instruction.opcode](vm);
    if (err)
      return err;
    prog->ptr++;
  }
  // Opcodes defined in loop
  else if (instruction.opcode == OP_JUMP_ABS)
    return vm_jump(vm, instruction.operand.as_word);
  else if (instruction.opcode == OP_JUMP_STACK)
  {
    data_t ret = {0};
    // Set prog->ptr to the word on top of the stack
    err_t err = vm_pop_word(vm, &ret);
    if (err)
      return err;
    return vm_jump(vm, ret.as_word);
  }
  else if (UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_JUMP_IF))
  {
    static_assert(DATA_TYPE_NIL == -1 && DATA_TYPE_WORD == 3,
                  "Code using OPCODE_DATA_TYPE for quick same type opcode "
                  "conversion may be out of date.");

    data_t datum = {0};
    // Here we add OP_POP_BYTE and the data_type_t of the opcode to
    // get the right OP_POP opcode.
    opcode_t pop_opcode =
        OPCODE_DATA_TYPE(instruction.opcode, OP_JUMP_IF) + OP_POP_BYTE;

    err_t err = POP_ROUTINES[pop_opcode](vm, &datum);
    if (err)
      return err;

    // If datum != 0 then jump, else go to the next instruction
    if (datum.as_word != 0)
      return vm_jump(vm, instruction.operand.as_word);
    ++prog->ptr;
  }
  else if (instruction.opcode == OP_CALL)
  {
    if (vm->call_stack.ptr >= vm->call_stack.max)
      return ERR_CALL_STACK_OVERFLOW;
    vm->call_stack.address_pointers[vm->call_stack.ptr++] = vm->program.ptr + 1;
    return vm_jump(vm, instruction.operand.as_word);
  }
  else if (instruction.opcode == OP_CALL_STACK)
  {
    if (vm->call_stack.ptr >= vm->call_stack.max)
      return ERR_CALL_STACK_OVERFLOW;
    vm->call_stack.address_pointers[vm->call_stack.ptr++] = vm->program.ptr + 1;
    data_t ret                                            = {0};
    err_t err = vm_pop_word(vm, &ret);
    if (err)
      return err;
    return vm_jump(vm, ret.as_word);
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
  else if (SIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_PRINT))
  {
    static_assert(DATA_TYPE_NIL == -1 && DATA_TYPE_WORD == 3,
                  "Code using OPCODE_DATA_TYPE for quick same type opcode "
                  "conversion may be out of date.");
    static_assert(OP_PRINT_SWORD - OP_PRINT_BYTE == 7,
                  "Implementation of OP_PRINT is out of date");
    /* 1) Pop
       2) Format
       3) Print
     */

    // 1) figure out what datum type to pop

    // type in [0, 7] representing [byte, sbyte, short, sshort, hword, shword,
    // word, sword]
    int type = OPCODE_DATA_TYPE(instruction.opcode, OP_PRINT);

    /* Byte and SByte -> POP_BYTE
       Short and SShort -> POP_SHORT
       HWord and SHword -> POP_HWORD
       Word and SWord -> POP_WORD
     */
    opcode_t pop_opcode = OP_POP_BYTE + (type / 2);

    data_t datum = {0};
    err_t err    = POP_ROUTINES[pop_opcode](vm, &datum);

    if (err)
      return err;

    // 2) create a format string for each datum type possible

    // TODO: Figure out a way to ensure the ordering of OP_PRINT_* is
    // exactly BYTE, SBYTE, SHORT, SSHORT, HWORD, SHWORD, WORD, SWORD
    // via static_assert

    // lookup table
    const char *format_strings[] = {
      "0x%X",
      "%c",
#if PRINT_HEX == 1
      "0x%X",
      "0x%X",
      "0x%X",
      "0x%X",
      "0x%lX",
      "0x%dX",
#else
      ("%" PRIu32),
      ("%" PRId32),
      ("%" PRIu64),
      ("%" PRId64),
#endif
    };

    // 3) Print datum using the format string given.
    printf(format_strings[type], datum);

    prog->ptr++;
  }
  else if (instruction.opcode == OP_HALT)
  {
    // Do nothing here.  Should be caught by callers of vm_execute
  }
  else
    return ERR_INVALID_OPCODE;
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
    fprintf(stdout, "[vm_execute_all]: Trace(Cycle %lu)\n", cycles);
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
  fprintf(stdout, "[%svm_execute_all%s]: Final VM state(Cycle %lu)\n",
          TERM_YELLOW, TERM_RESET, cycles);
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

#define VM_MALLOC_CONSTR(TYPE, TYPE_CAP)                          \
  err_t vm_malloc_##TYPE(vm_t *vm, word_t n)                      \
  {                                                               \
    page_t *page = heap_allocate(&vm->heap, n * TYPE_CAP##_SIZE); \
    return vm_push_word(vm, DWORD((word_t)page));                 \
  }

VM_MALLOC_CONSTR(byte, BYTE)
VM_MALLOC_CONSTR(short, SHORT)
VM_MALLOC_CONSTR(hword, HWORD)
VM_MALLOC_CONSTR(word, WORD)

#define VM_MSET_CONSTR(TYPE, TYPE_CAP)                     \
  err_t vm_mset_##TYPE(vm_t *vm, word_t nth)               \
  {                                                        \
    data_t object = {0};                                   \
    err_t err     = vm_pop_##TYPE(vm, &object);            \
    if (err)                                               \
      return err;                                          \
    data_t ptr = {0};                                      \
    err        = vm_pop_word(vm, &ptr);                    \
    if (err)                                               \
      return err;                                          \
    page_t *page = (page_t *)ptr.as_word;                  \
    if (nth >= (page->available / TYPE_CAP##_SIZE))        \
      return ERR_OUT_OF_BOUNDS;                            \
    DARR_AT(TYPE##_t, page->data, nth) = object.as_##TYPE; \
    return ERR_OK;                                         \
  }

VM_MSET_CONSTR(byte, BYTE)
VM_MSET_CONSTR(short, SHORT)
VM_MSET_CONSTR(hword, HWORD)
VM_MSET_CONSTR(word, WORD)

#define VM_MGET_CONSTR(TYPE, TYPE_CAP)                             \
  err_t vm_mget_##TYPE(vm_t *vm, word_t n)                         \
  {                                                                \
    data_t ptr = {0};                                              \
    err_t err  = vm_pop_word(vm, &ptr);                            \
    if (err)                                                       \
      return err;                                                  \
    page_t *page = (page_t *)ptr.as_word;                          \
    if (n >= (page->available / TYPE_CAP##_SIZE))                  \
      return ERR_OUT_OF_BOUNDS;                                    \
    else if (vm->stack.ptr + TYPE_CAP##_SIZE >= vm->stack.max)     \
      return ERR_STACK_OVERFLOW;                                   \
    memcpy(vm->stack.data + vm->stack.ptr,                         \
           page->data + (n * (TYPE_CAP##_SIZE)), TYPE_CAP##_SIZE); \
    vm->stack.ptr += TYPE_CAP##_SIZE;                              \
    return ERR_OK;                                                 \
  }

VM_MGET_CONSTR(byte, BYTE)
VM_MGET_CONSTR(short, SHORT)
VM_MGET_CONSTR(hword, HWORD)
VM_MGET_CONSTR(word, WORD)

// TODO: rename this to something more appropriate
#define VM_MEMORY_STACK_CONSTR(ACTION, TYPE)    \
  err_t vm_##ACTION##_stack_##TYPE(vm_t *vm)    \
  {                                             \
    data_t n  = {0};                            \
    err_t err = vm_pop_word(vm, &n);            \
    if (err)                                    \
      return err;                               \
    return vm_##ACTION##_##TYPE(vm, n.as_word); \
  }

VM_MEMORY_STACK_CONSTR(malloc, byte)
VM_MEMORY_STACK_CONSTR(malloc, short)
VM_MEMORY_STACK_CONSTR(malloc, hword)
VM_MEMORY_STACK_CONSTR(malloc, word)
VM_MEMORY_STACK_CONSTR(mset, byte)
VM_MEMORY_STACK_CONSTR(mset, short)
VM_MEMORY_STACK_CONSTR(mset, hword)
VM_MEMORY_STACK_CONSTR(mset, word)
VM_MEMORY_STACK_CONSTR(mget, byte)
VM_MEMORY_STACK_CONSTR(mget, short)
VM_MEMORY_STACK_CONSTR(mget, hword)
VM_MEMORY_STACK_CONSTR(mget, word)

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
