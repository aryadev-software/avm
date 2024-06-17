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

#include "./runtime.h"

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

static_assert(NUMBER_OF_OPCODES == 99, "vm_execute: Out of date");

static_assert(DATA_TYPE_NIL == -1 && DATA_TYPE_WORD == 2,
              "Code using OPCODE_DATA_TYPE for quick same type opcode "
              "conversion may be out of date.");

static_assert(OP_PRINT_LONG - OP_PRINT_BYTE == 5,
              "Implementation of OP_PRINT is out of date");

err_t vm_execute(vm_t *vm)
{
  struct Program *prog = &vm->program;
  prog_t program_data  = prog->data;
  if (prog->ptr >= program_data.count)
    return ERR_END_OF_PROGRAM;
  inst_t instruction = program_data.instructions[prog->ptr];

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
    data_t datum = {0};

    // Here we add OP_POP_BYTE and the data_type_t of the opcode to
    // get the right typed OP_POP opcode.
    opcode_t pop_opcode =
        OPCODE_DATA_TYPE(instruction.opcode, OP_JUMP_IF) + OP_POP_BYTE;

    err_t err = POP_ROUTINES[pop_opcode](vm, &datum);

    if (err)
      return err;

    // If datum != 0 then jump, else go to the next instruction
    if (datum.as_word != 0)
      return vm_jump(vm, instruction.operand.as_word);
    else
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
    /* 1) Pop
       2) Format
       3) Print
     */

    // 1) figure out what datum type to pop

    // type in [0, 5] representing [byte, char, hword, int, word,
    // long]
    int type = OPCODE_DATA_TYPE(instruction.opcode, OP_PRINT);

    /* Byte and Char -> POP_BYTE
       HWord and Int -> POP_HWORD
       Word and Long -> POP_WORD
     */
    opcode_t pop_opcode = OP_POP_BYTE + (type / 2);

    data_t datum = {0};
    err_t err    = POP_ROUTINES[pop_opcode](vm, &datum);

    if (err)
      return err;

    // 2) create a format string for each datum type possible

    // TODO: Figure out a way to ensure the ordering of OP_PRINT_* is
    // exactly BYTE, CHAR, HWORD, INTEGER, WORD, LONG.  Perhaps via
    // static_assert

    // lookup table
    const char *format_strings[] = {
      "0x%x",
      "%c",
#if PRINT_HEX == 1
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
  registers_t prev_registers = vm->registers;
  size_t prev_sptr           = 0;
  size_t prev_pages          = 0;
  size_t prev_cptr           = 0;
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
    if (prev_pages != vm->heap.pages)
    {
      vm_print_heap(vm, stdout);
      prev_pages = vm->heap.pages;
      fputs("------------------------------------------------------------------"
            "----"
            "----------\n",
            stdout);
    }
    if (memcmp(&prev_registers, &vm->registers, sizeof(darr_t)) != 0)
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

err_t vm_push_hword(vm_t *vm, data_t f)
{
  if (vm->stack.ptr + HWORD_SIZE >= vm->stack.max)
    return ERR_STACK_OVERFLOW;
  byte_t bytes[HWORD_SIZE] = {0};
  convert_hword_to_bytes(f.as_hword, bytes);
  for (size_t i = 0; i < HWORD_SIZE; ++i)
  {
    byte_t b  = bytes[HWORD_SIZE - i - 1];
    err_t err = vm_push_byte(vm, DBYTE(b));
    if (err)
      return err;
  }
  return ERR_OK;
}

err_t vm_push_word(vm_t *vm, data_t w)
{
  if (vm->stack.ptr + WORD_SIZE >= vm->stack.max)
    return ERR_STACK_OVERFLOW;
  byte_t bytes[WORD_SIZE] = {0};
  convert_word_to_bytes(w.as_word, bytes);
  for (size_t i = 0; i < WORD_SIZE; ++i)
  {
    byte_t b  = bytes[WORD_SIZE - i - 1];
    err_t err = vm_push_byte(vm, DBYTE(b));
    if (err)
      return err;
  }
  return ERR_OK;
}

err_t vm_push_byte_register(vm_t *vm, word_t reg)
{
  if (reg > vm->registers.used)
    return ERR_INVALID_REGISTER_BYTE;

  // Interpret each word based register as 8 byte registers
  byte_t b = vm->registers.data[reg];

  return vm_push_byte(vm, DBYTE(b));
}

err_t vm_push_hword_register(vm_t *vm, word_t reg)
{
  if (reg > (vm->registers.used / HWORD_SIZE))
    return ERR_INVALID_REGISTER_HWORD;
  // Interpret the bytes at point reg * HWORD_SIZE as an hword
  hword_t hw = *(hword_t *)(vm->registers.data + (reg * HWORD_SIZE));
  return vm_push_hword(vm, DHWORD(hw));
}

err_t vm_push_word_register(vm_t *vm, word_t reg)
{
  if (reg > (vm->registers.used / WORD_SIZE))
    return ERR_INVALID_REGISTER_WORD;
  return vm_push_word(vm, DWORD(VM_NTH_REGISTER(vm->registers, reg)));
}

err_t vm_mov_byte(vm_t *vm, word_t reg)
{
  if (reg >= vm->registers.used)
  {
    // Expand capacity
    darr_ensure_capacity(&vm->registers, reg - vm->registers.used);
    vm->registers.used = MAX(vm->registers.used, reg + 1);
  }
  data_t ret = {0};
  err_t err  = vm_pop_byte(vm, &ret);
  if (err)
    return err;
  vm->registers.data[reg] = ret.as_byte;
  return ERR_OK;
}

err_t vm_mov_hword(vm_t *vm, word_t reg)
{
  if (reg >= (vm->registers.used / HWORD_SIZE))
  {
    // Expand capacity till we can ensure that this is a valid
    // register to use

    // Number of hwords needed ontop of what is allocated:
    const size_t hwords = (reg - (vm->registers.used / HWORD_SIZE));
    // Number of bytes needed ontop of what is allocated
    const size_t diff = (hwords + 1) * HWORD_SIZE;

    darr_ensure_capacity(&vm->registers, diff);
    vm->registers.used = MAX(vm->registers.used, (reg + 1) * HWORD_SIZE);
  }
  data_t ret = {0};
  err_t err  = vm_pop_hword(vm, &ret);
  if (err)
    return err;
  // Here we treat vm->registers as a set of hwords
  hword_t *hword_ptr = (hword_t *)(vm->registers.data + (reg * HWORD_SIZE));
  *hword_ptr         = ret.as_hword;
  return ERR_OK;
}

err_t vm_mov_word(vm_t *vm, word_t reg)
{
  if (reg >= (vm->registers.used / WORD_SIZE))
  {
    // Number of hwords needed ontop of what is allocated:
    const size_t words = (reg - (vm->registers.used / WORD_SIZE));
    // Number of bytes needed ontop of what is allocated
    const size_t diff = (words + 1) * WORD_SIZE;

    darr_ensure_capacity(&vm->registers, diff);
    vm->registers.used = MAX(vm->registers.used, (reg + 1) * WORD_SIZE);
  }
  else if (vm->stack.ptr < WORD_SIZE)
    return ERR_STACK_UNDERFLOW;
  data_t ret = {0};
  err_t err  = vm_pop_word(vm, &ret);
  if (err)
    return err;
  ((word_t *)(vm->registers.data))[reg] = ret.as_word;
  return ERR_OK;
}

err_t vm_dup_byte(vm_t *vm, word_t w)
{
  if (vm->stack.ptr < w + 1)
    return ERR_STACK_UNDERFLOW;
  return vm_push_byte(vm, DBYTE(vm->stack.data[vm->stack.ptr - 1 - w]));
}

err_t vm_dup_hword(vm_t *vm, word_t w)
{
  if (vm->stack.ptr < HWORD_SIZE * (w + 1))
    return ERR_STACK_UNDERFLOW;
  byte_t bytes[HWORD_SIZE] = {0};
  for (size_t i = 0; i < HWORD_SIZE; ++i)
    bytes[HWORD_SIZE - i - 1] =
        vm->stack.data[vm->stack.ptr - (HWORD_SIZE * (w + 1)) + i];
  return vm_push_hword(vm, DHWORD(convert_bytes_to_hword(bytes)));
}

err_t vm_dup_word(vm_t *vm, word_t w)
{
  if (vm->stack.ptr < WORD_SIZE * (w + 1))
    return ERR_STACK_UNDERFLOW;
  byte_t bytes[WORD_SIZE] = {0};
  for (size_t i = 0; i < WORD_SIZE; ++i)
    bytes[WORD_SIZE - i - 1] =
        vm->stack.data[vm->stack.ptr - (WORD_SIZE * (w + 1)) + i];
  return vm_push_word(vm, DWORD(convert_bytes_to_word(bytes)));
}

err_t vm_malloc_byte(vm_t *vm, word_t n)
{
  page_t *page = heap_allocate(&vm->heap, n);
  return vm_push_word(vm, DWORD((word_t)page));
}

err_t vm_malloc_hword(vm_t *vm, word_t n)
{
  page_t *page = heap_allocate(&vm->heap, n * HWORD_SIZE);
  return vm_push_word(vm, DWORD((word_t)page));
}

err_t vm_malloc_word(vm_t *vm, word_t n)
{
  page_t *page = heap_allocate(&vm->heap, n * WORD_SIZE);
  return vm_push_word(vm, DWORD((word_t)page));
}

err_t vm_mset_byte(vm_t *vm, word_t nth)
{
  // Stack layout should be [BYTE, PTR]
  data_t byte = {0};
  err_t err   = vm_pop_byte(vm, &byte);
  if (err)
    return err;
  data_t ptr = {0};
  err        = vm_pop_word(vm, &ptr);
  if (err)
    return err;

  page_t *page = (page_t *)ptr.as_word;
  if (nth >= page->available)
    return ERR_OUT_OF_BOUNDS;
  page->data[nth] = byte.as_byte;

  return ERR_OK;
}

err_t vm_mset_hword(vm_t *vm, word_t nth)
{
  // Stack layout should be [HWORD, PTR]
  data_t byte = {0};
  err_t err   = vm_pop_hword(vm, &byte);
  if (err)
    return err;
  data_t ptr = {0};
  err        = vm_pop_word(vm, &ptr);
  if (err)
    return err;

  page_t *page = (page_t *)ptr.as_word;
  if (nth >= (page->available / HWORD_SIZE))
    return ERR_OUT_OF_BOUNDS;
  ((hword_t *)page->data)[nth] = byte.as_hword;

  return ERR_OK;
}

err_t vm_mset_word(vm_t *vm, word_t nth)
{
  // Stack layout should be [WORD, PTR]
  data_t byte = {0};
  err_t err   = vm_pop_word(vm, &byte);
  if (err)
    return err;
  data_t ptr = {0};
  err        = vm_pop_word(vm, &ptr);
  if (err)
    return err;

  page_t *page = (page_t *)ptr.as_word;
  if (nth >= (page->available / WORD_SIZE))
    return ERR_OUT_OF_BOUNDS;
  ((word_t *)page->data)[nth] = byte.as_word;

  return ERR_OK;
}

err_t vm_mget_byte(vm_t *vm, word_t n)
{
  // Stack layout should be [PTR]
  data_t ptr = {0};
  err_t err  = vm_pop_word(vm, &ptr);
  if (err)
    return err;
  page_t *page = (page_t *)ptr.as_word;
  if (n >= page->available)
    return ERR_OUT_OF_BOUNDS;
  return vm_push_byte(vm, DBYTE(page->data[n]));
}

err_t vm_mget_hword(vm_t *vm, word_t n)
{
  // Stack layout should be [PTR]
  data_t ptr = {0};
  err_t err  = vm_pop_word(vm, &ptr);
  if (err)
    return err;
  page_t *page = (page_t *)ptr.as_word;
  if (n >= (page->available / HWORD_SIZE))
    return ERR_OUT_OF_BOUNDS;
  return vm_push_hword(vm, DHWORD(((hword_t *)page->data)[n]));
}

err_t vm_mget_word(vm_t *vm, word_t n)
{
  // Stack layout should be [PTR]
  data_t ptr = {0};
  err_t err  = vm_pop_word(vm, &ptr);
  if (err)
    return err;
  printf("%lx\n", ptr.as_word);
  page_t *page = (page_t *)ptr.as_word;
  if (n >= (page->available / WORD_SIZE))
    return ERR_OUT_OF_BOUNDS;
  return vm_push_word(vm, DWORD(((word_t *)page->data)[n]));
}

err_t vm_pop_byte(vm_t *vm, data_t *ret)
{
  if (vm->stack.ptr == 0)
    return ERR_STACK_UNDERFLOW;
  *ret = DBYTE(vm->stack.data[--vm->stack.ptr]);
  return ERR_OK;
}

err_t vm_pop_hword(vm_t *vm, data_t *ret)
{
  if (vm->stack.ptr < HWORD_SIZE)
    return ERR_STACK_UNDERFLOW;
  byte_t bytes[HWORD_SIZE] = {0};
  for (size_t i = 0; i < HWORD_SIZE; ++i)
  {
    data_t b = {0};
    vm_pop_byte(vm, &b);
    bytes[i] = b.as_byte;
  }
  *ret = DHWORD(convert_bytes_to_hword(bytes));
  return ERR_OK;
}

err_t vm_pop_word(vm_t *vm, data_t *ret)
{
  if (vm->stack.ptr < WORD_SIZE)
    return ERR_STACK_UNDERFLOW;
  byte_t bytes[WORD_SIZE] = {0};
  for (size_t i = 0; i < WORD_SIZE; ++i)
  {
    data_t b = {0};
    vm_pop_byte(vm, &b);
    bytes[i] = b.as_byte;
  }
  *ret = DWORD(convert_bytes_to_word(bytes));
  return ERR_OK;
}

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
VM_MEMORY_STACK_CONSTR(malloc, hword)
VM_MEMORY_STACK_CONSTR(malloc, word)
VM_MEMORY_STACK_CONSTR(mset, byte)
VM_MEMORY_STACK_CONSTR(mset, hword)
VM_MEMORY_STACK_CONSTR(mset, word)
VM_MEMORY_STACK_CONSTR(mget, byte)
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
VM_SAME_TYPE(or, |, hword, HWORD)
VM_SAME_TYPE(or, |, word, WORD)
VM_SAME_TYPE(and, &, byte, BYTE)
VM_SAME_TYPE(and, &, hword, HWORD)
VM_SAME_TYPE(and, &, word, WORD)
VM_SAME_TYPE(xor, ^, byte, BYTE)
VM_SAME_TYPE(xor, ^, hword, HWORD)
VM_SAME_TYPE(xor, ^, word, WORD)

VM_SAME_TYPE(plus, +, byte, BYTE)
VM_SAME_TYPE(plus, +, hword, HWORD)
VM_SAME_TYPE(plus, +, word, WORD)

VM_SAME_TYPE(sub, -, byte, BYTE)
VM_SAME_TYPE(sub, -, hword, HWORD)
VM_SAME_TYPE(sub, -, word, WORD)

VM_SAME_TYPE(mult, *, byte, BYTE)
VM_SAME_TYPE(mult, *, hword, HWORD)
VM_SAME_TYPE(mult, *, word, WORD)

VM_COMPARATOR_TYPE(eq, ==, byte, byte)
VM_COMPARATOR_TYPE(eq, ==, byte, char)
VM_COMPARATOR_TYPE(eq, ==, hword, hword)
VM_COMPARATOR_TYPE(eq, ==, hword, int)
VM_COMPARATOR_TYPE(eq, ==, word, word)
VM_COMPARATOR_TYPE(eq, ==, word, long)

VM_COMPARATOR_TYPE(lt, <, byte, byte)
VM_COMPARATOR_TYPE(lt, <, byte, char)
VM_COMPARATOR_TYPE(lt, <, hword, hword)
VM_COMPARATOR_TYPE(lt, <, hword, int)
VM_COMPARATOR_TYPE(lt, <, word, word)
VM_COMPARATOR_TYPE(lt, <, word, long)

VM_COMPARATOR_TYPE(lte, <=, byte, byte)
VM_COMPARATOR_TYPE(lte, <=, byte, char)
VM_COMPARATOR_TYPE(lte, <=, hword, hword)
VM_COMPARATOR_TYPE(lte, <=, hword, int)
VM_COMPARATOR_TYPE(lte, <=, word, word)
VM_COMPARATOR_TYPE(lte, <=, word, long)

VM_COMPARATOR_TYPE(gt, >, byte, byte)
VM_COMPARATOR_TYPE(gt, >, byte, char)
VM_COMPARATOR_TYPE(gt, >, hword, hword)
VM_COMPARATOR_TYPE(gt, >, hword, int)
VM_COMPARATOR_TYPE(gt, >, word, word)
VM_COMPARATOR_TYPE(gt, >, word, long)

VM_COMPARATOR_TYPE(gte, >=, byte, byte)
VM_COMPARATOR_TYPE(gte, >=, byte, char)
VM_COMPARATOR_TYPE(gte, >=, hword, hword)
VM_COMPARATOR_TYPE(gte, >=, hword, int)
VM_COMPARATOR_TYPE(gte, >=, word, word)
VM_COMPARATOR_TYPE(gte, >=, word, long)
