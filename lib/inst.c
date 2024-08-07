/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Implementation of bytecode for instructions
 */

#include "./inst.h"
#include "lib/base.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

const char *opcode_as_cstr(opcode_t code)
{
  switch (code)
  {
  case OP_NOOP:
    return "NOOP";
  case OP_HALT:
    return "HALT";
  case OP_PUSH_BYTE:
    return "PUSH_BYTE";
  case OP_PUSH_SHORT:
    return "PUSH_SHORT";
  case OP_PUSH_HWORD:
    return "PUSH_HWORD";
  case OP_PUSH_WORD:
    return "PUSH_WORD";
  case OP_POP_BYTE:
    return "POP_BYTE";
  case OP_POP_SHORT:
    return "POP_SHORT";
  case OP_POP_HWORD:
    return "POP_HWORD";
  case OP_POP_WORD:
    return "POP_WORD";
  case OP_PUSH_REGISTER_BYTE:
    return "PUSH_REGISTER_BYTE";
  case OP_PUSH_REGISTER_SHORT:
    return "PUSH_REGISTER_SHORT";
  case OP_PUSH_REGISTER_HWORD:
    return "PUSH_REGISTER_HWORD";
  case OP_PUSH_REGISTER_WORD:
    return "PUSH_REGISTER_WORD";
  case OP_MOV_BYTE:
    return "MOV_BYTE";
  case OP_MOV_SHORT:
    return "MOV_SHORT";
  case OP_MOV_HWORD:
    return "MOV_HWORD";
  case OP_MOV_WORD:
    return "MOV_WORD";
  case OP_DUP_BYTE:
    return "DUP_BYTE";
  case OP_DUP_SHORT:
    return "DUP_SHORT";
  case OP_DUP_HWORD:
    return "DUP_HWORD";
  case OP_DUP_WORD:
    return "DUP_WORD";
  case OP_MALLOC_BYTE:
    return "MALLOC_BYTE";
  case OP_MALLOC_SHORT:
    return "MALLOC_SHORT";
  case OP_MALLOC_HWORD:
    return "MALLOC_HWORD";
  case OP_MALLOC_WORD:
    return "MALLOC_WORD";
  case OP_MSET_BYTE:
    return "MSET_BYTE";
  case OP_MSET_SHORT:
    return "MSET_SHORT";
  case OP_MSET_HWORD:
    return "MSET_HWORD";
  case OP_MSET_WORD:
    return "MSET_WORD";
  case OP_MGET_BYTE:
    return "MGET_BYTE";
  case OP_MGET_SHORT:
    return "MGET_SHORT";
  case OP_MGET_HWORD:
    return "MGET_HWORD";
  case OP_MGET_WORD:
    return "MGET_WORD";
  case OP_MDELETE:
    return "MDELETE";
  case OP_MSIZE:
    return "MSIZE";
  case OP_NOT_BYTE:
    return "NOT_BYTE";
  case OP_NOT_SHORT:
    return "NOT_SHORT";
  case OP_NOT_HWORD:
    return "NOT_HWORD";
  case OP_NOT_WORD:
    return "NOT_WORD";
  case OP_OR_BYTE:
    return "OR_BYTE";
  case OP_OR_SHORT:
    return "OR_SHORT";
  case OP_OR_HWORD:
    return "OR_HWORD";
  case OP_OR_WORD:
    return "OR_WORD";
  case OP_AND_BYTE:
    return "AND_BYTE";
  case OP_AND_SHORT:
    return "AND_SHORT";
  case OP_AND_HWORD:
    return "AND_HWORD";
  case OP_AND_WORD:
    return "AND_WORD";
  case OP_XOR_BYTE:
    return "XOR_BYTE";
  case OP_XOR_SHORT:
    return "XOR_SHORT";
  case OP_XOR_HWORD:
    return "XOR_HWORD";
  case OP_XOR_WORD:
    return "XOR_WORD";
  case OP_EQ_BYTE:
    return "EQ_BYTE";
  case OP_EQ_SHORT:
    return "EQ_SHORT";
  case OP_EQ_HWORD:
    return "EQ_HWORD";
  case OP_EQ_WORD:
    return "EQ_WORD";
  case OP_PLUS_BYTE:
    return "PLUS_BYTE";
  case OP_PLUS_SHORT:
    return "PLUS_SHORT";
  case OP_PLUS_HWORD:
    return "PLUS_HWORD";
  case OP_PLUS_WORD:
    return "PLUS_WORD";
  case OP_SUB_BYTE:
    return "SUB_BYTE";
  case OP_SUB_SHORT:
    return "SUB_SHORT";
  case OP_SUB_HWORD:
    return "SUB_HWORD";
  case OP_SUB_WORD:
    return "SUB_WORD";
  case OP_MULT_BYTE:
    return "MULT_BYTE";
  case OP_MULT_SHORT:
    return "MULT_SHORT";
  case OP_MULT_HWORD:
    return "MULT_HWORD";
  case OP_MULT_WORD:
    return "MULT_WORD";
  case OP_LT_BYTE:
    return "LT_BYTE";
  case OP_LT_SBYTE:
    return "LT_SBYTE";
  case OP_LT_SHORT:
    return "LT_SHORT";
  case OP_LT_SSHORT:
    return "LT_SSHORT";
  case OP_LT_HWORD:
    return "LT_HWORD";
  case OP_LT_SHWORD:
    return "LT_SHWORD";
  case OP_LT_WORD:
    return "LT_WORD";
  case OP_LT_SWORD:
    return "LT_SWORD";
  case OP_LTE_BYTE:
    return "LTE_BYTE";
  case OP_LTE_SBYTE:
    return "LTE_SBYTE";
  case OP_LTE_SHORT:
    return "LTE_SHORT";
  case OP_LTE_SSHORT:
    return "LTE_SSHORT";
  case OP_LTE_HWORD:
    return "LTE_HWORD";
  case OP_LTE_SHWORD:
    return "LTE_SHWORD";
  case OP_LTE_WORD:
    return "LTE_WORD";
  case OP_LTE_SWORD:
    return "LTE_SWORD";
  case OP_GT_BYTE:
    return "GT_BYTE";
  case OP_GT_SBYTE:
    return "GT_SBYTE";
  case OP_GT_SHORT:
    return "GT_SHORT";
  case OP_GT_SSHORT:
    return "GT_SSHORT";
  case OP_GT_HWORD:
    return "GT_HWORD";
  case OP_GT_SHWORD:
    return "GT_SHWORD";
  case OP_GT_WORD:
    return "GT_WORD";
  case OP_GT_SWORD:
    return "GT_SWORD";
  case OP_GTE_BYTE:
    return "GTE_BYTE";
  case OP_GTE_SBYTE:
    return "GTE_SBYTE";
  case OP_GTE_SHORT:
    return "GTE_SHORT";
  case OP_GTE_SSHORT:
    return "GTE_SSHORT";
  case OP_GTE_HWORD:
    return "GTE_HWORD";
  case OP_GTE_SHWORD:
    return "GTE_SHWORD";
  case OP_GTE_WORD:
    return "GTE_WORD";
  case OP_GTE_SWORD:
    return "GTE_SWORD";
  case OP_PRINT_BYTE:
    return "PRINT_BYTE";
  case OP_PRINT_SBYTE:
    return "PRINT_SBYTE";
  case OP_PRINT_SHORT:
    return "PRINT_SHORT";
  case OP_PRINT_SSHORT:
    return "PRINT_SSHORT";
  case OP_PRINT_HWORD:
    return "PRINT_HWORD";
  case OP_PRINT_SHWORD:
    return "PRINT_SHWORD";
  case OP_PRINT_WORD:
    return "PRINT_WORD";
  case OP_PRINT_SWORD:
    return "PRINT_SWORD";
  case OP_JUMP_ABS:
    return "JUMP_ABS";
  case OP_JUMP_IF_BYTE:
    return "JUMP_IF_BYTE";
  case OP_JUMP_IF_SHORT:
    return "JUMP_IF_SHORT";
  case OP_JUMP_IF_HWORD:
    return "JUMP_IF_HWORD";
  case OP_JUMP_IF_WORD:
    return "JUMP_IF_WORD";
  case OP_CALL:
    return "CALL";
  case OP_RET:
    return "RET";
  case NUMBER_OF_OPCODES:
    return "";
  }
  return "";
}

void data_print(data_t datum, data_type_t type, FILE *fp)
{
  switch (type)
  {
  case DATA_TYPE_NIL:
    break;
  case DATA_TYPE_BYTE:
    fprintf(fp, "%X", datum.as_byte);
    break;
  case DATA_TYPE_SHORT:
    fprintf(fp, "%X", datum.as_short);
    break;
  case DATA_TYPE_HWORD:
    fprintf(fp, "%X", datum.as_hword);
    break;
  case DATA_TYPE_WORD:
    fprintf(fp, "%lX", datum.as_word);
    break;
  }
}

void inst_print(inst_t instruction, FILE *fp)
{
  static_assert(NUMBER_OF_OPCODES == 115, "inst_print: Out of date");
  fprintf(fp, "%s(", opcode_as_cstr(instruction.opcode));
  if (UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_PUSH))
  {
    data_type_t type = (data_type_t)instruction.opcode;
    fprintf(fp, "datum=0x");
    data_print(instruction.operand, type, fp);
  }
  else if (UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_PUSH_REGISTER) ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_MOV))
  {
    fprintf(fp, "reg=0x");
    data_print(instruction.operand, DATA_TYPE_BYTE, fp);
  }
  else if (UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_DUP))
  {
    fprintf(fp, "n=0x%lX", instruction.operand.as_word);
  }
  else if (instruction.opcode == OP_JUMP_ABS ||
           UNSIGNED_OPCODE_IS_TYPE(instruction.opcode, OP_JUMP_IF) ||
           instruction.opcode == OP_CALL)
  {
    fprintf(fp, "address=0x");
    data_print(instruction.operand, DATA_TYPE_WORD, fp);
  }
  fprintf(fp, ")");
}

size_t opcode_bytecode_size(opcode_t opcode)
{
  static_assert(NUMBER_OF_OPCODES == 115, "inst_bytecode_size: Out of date");
  size_t size = 1; // for opcode
  if (UNSIGNED_OPCODE_IS_TYPE(opcode, OP_PUSH))
  {
    if (opcode == OP_PUSH_BYTE)
      ++size;
    else if (opcode == OP_PUSH_HWORD)
      size += HWORD_SIZE;
    else if (opcode == OP_PUSH_WORD)
      size += WORD_SIZE;
  }
  else if (UNSIGNED_OPCODE_IS_TYPE(opcode, OP_PUSH_REGISTER) ||
           UNSIGNED_OPCODE_IS_TYPE(opcode, OP_MOV) ||
           UNSIGNED_OPCODE_IS_TYPE(opcode, OP_DUP) ||
           UNSIGNED_OPCODE_IS_TYPE(opcode, OP_MALLOC) ||
           UNSIGNED_OPCODE_IS_TYPE(opcode, OP_MSET) ||
           UNSIGNED_OPCODE_IS_TYPE(opcode, OP_MGET) ||
           UNSIGNED_OPCODE_IS_TYPE(opcode, OP_JUMP_IF) ||
           opcode == OP_JUMP_ABS || opcode == OP_CALL)
    size += WORD_SIZE;
  return size;
}

size_t inst_write_bytecode(inst_t inst, byte_t *bytes)
{
  static_assert(NUMBER_OF_OPCODES == 115, "inst_write_bytecode: Out of date");

  bytes[0]       = inst.opcode;
  size_t written = 1;
  // Then append 0 or more operands
  data_type_t to_append = DATA_TYPE_NIL;
  if (UNSIGNED_OPCODE_IS_TYPE(inst.opcode, OP_PUSH))
    to_append = OPCODE_DATA_TYPE(inst.opcode, OP_PUSH);
  else if (UNSIGNED_OPCODE_IS_TYPE(inst.opcode, OP_PUSH_REGISTER) ||
           UNSIGNED_OPCODE_IS_TYPE(inst.opcode, OP_DUP) ||
           UNSIGNED_OPCODE_IS_TYPE(inst.opcode, OP_MOV) ||
           UNSIGNED_OPCODE_IS_TYPE(inst.opcode, OP_DUP) ||
           UNSIGNED_OPCODE_IS_TYPE(inst.opcode, OP_JUMP_IF) ||
           inst.opcode == OP_JUMP_ABS || inst.opcode == OP_CALL)
    to_append = DATA_TYPE_WORD;

  switch (to_append)
  {
  case DATA_TYPE_NIL:
    break;
  case DATA_TYPE_BYTE: {
    bytes[1] = inst.operand.as_byte;
    written += 1;
    break;
  }
  case DATA_TYPE_SHORT: {
    convert_short_to_bytes(inst.operand.as_short, bytes + 1);
    written += SHORT_SIZE;
    break;
  }
  case DATA_TYPE_HWORD: {
    convert_hword_to_bytes(inst.operand.as_hword, bytes + 1);
    written += HWORD_SIZE;
    break;
  }
  case DATA_TYPE_WORD: {
    convert_word_to_bytes(inst.operand.as_word, bytes + 1);
    written += WORD_SIZE;
    break;
  }
  }
  return written;
}

bool read_type_from_darr(byte_t *bytes, size_t size, data_type_t type,
                         data_t *data)
{
  data_t datum = {0};
  switch (type)
  {
  case DATA_TYPE_NIL:
    break;
  case DATA_TYPE_BYTE:
    if (size == 0)
      return false;
    datum = DBYTE(bytes[0]);
    break;
  case DATA_TYPE_SHORT:
    if (size < SHORT_SIZE)
      return false;
    short s = convert_bytes_to_short(bytes);
    datum   = DSHORT(s);
    break;
  case DATA_TYPE_HWORD:
    if (size < HWORD_SIZE)
      return false;
    hword_t u = convert_bytes_to_hword(bytes);
    datum     = DHWORD(u);
    break;
  case DATA_TYPE_WORD:
    if (size < WORD_SIZE)
      return false;
    word_t w = convert_bytes_to_word(bytes);
    datum    = DWORD(w);
    break;
  default:
    return false;
  }
  *data = datum;
  return true;
}

int inst_read_bytecode(inst_t *ptr, byte_t *bytes, size_t size_bytes)
{
  static_assert(NUMBER_OF_OPCODES == 115, "inst_read_bytecode: Out of date");

  opcode_t opcode = *(bytes++);
  if (opcode >= NUMBER_OF_OPCODES || opcode < OP_NOOP)
    return READ_ERR_INVALID_OPCODE;

  inst_t inst = {opcode, {0}};
  --size_bytes;

  bool success = true;

  // Read operands
  if (UNSIGNED_OPCODE_IS_TYPE(opcode, OP_PUSH))
    success = read_type_from_darr(bytes, size_bytes, (data_type_t)opcode,
                                  &inst.operand);
  // Read operand as a word
  else if (UNSIGNED_OPCODE_IS_TYPE(opcode, OP_PUSH_REGISTER) ||
           UNSIGNED_OPCODE_IS_TYPE(opcode, OP_MOV) ||
           UNSIGNED_OPCODE_IS_TYPE(opcode, OP_DUP) ||
           UNSIGNED_OPCODE_IS_TYPE(opcode, OP_JUMP_IF) ||
           opcode == OP_JUMP_ABS || opcode == OP_CALL)
    success =
        read_type_from_darr(bytes, size_bytes, DATA_TYPE_WORD, &inst.operand);
  else
  {
    // Instruction doesn't take operands
  }

  if (success)
  {
    *ptr = inst;
    return (int)(size_bytes);
  }
  else
    return READ_ERR_OPERAND_NO_FIT;
}

static_assert(sizeof(prog_t) == (WORD_SIZE * 2) + sizeof(inst_t *),
              "prog_{write|read}_* is out of date");

size_t prog_bytecode_size(prog_t program)
{
  size_t size = WORD_SIZE * 2;
  for (size_t i = 0; i < program.count; ++i)
    size += opcode_bytecode_size(program.instructions[i].opcode);
  return size;
}

size_t prog_write_bytecode(prog_t program, byte_t *bytes, size_t size_bytes)
{
  if (size_bytes < PROG_HEADER_SIZE || prog_bytecode_size(program) < size_bytes)
    return 0;
  size_t b_iter = 0;
  // Write program header i.e. the start and count
  convert_word_to_bytes(program.start_address, bytes);
  b_iter += WORD_SIZE;
  convert_word_to_bytes(program.count, bytes + b_iter);
  b_iter += WORD_SIZE;

  // Write instructions
  size_t p_iter = 0;
  for (; p_iter < program.count && b_iter < size_bytes; ++p_iter)
  {
    size_t written =
        inst_write_bytecode(program.instructions[p_iter], bytes + b_iter);
    if (written == 0)
      return 0;
    b_iter += written;
  }

  return b_iter;
}

size_t prog_read_header(prog_t *prog, byte_t *bytes, size_t size_bytes)
{
  if (size_bytes < PROG_HEADER_SIZE)
    return 0;
  prog->start_address = convert_bytes_to_word(bytes);
  prog->count         = convert_bytes_to_word(bytes + WORD_SIZE);

  if (prog->start_address >= prog->count)
    return 0;
  return PROG_HEADER_SIZE;
}

read_err_prog_t prog_read_instructions(prog_t *program, size_t *size_bytes_read,
                                       byte_t *bytes, size_t size_bytes)
{
  // If no count then must be empty
  if (program->count == 0)
    return (read_err_prog_t){0};

  size_t program_iter = 0, byte_iter = 0;
  for (; program_iter < program->count && byte_iter < size_bytes;
       ++program_iter)
  {
    inst_t inst = {0};
    int bytes_read =
        inst_read_bytecode(&inst, bytes + byte_iter, size_bytes - byte_iter);
    if (bytes_read < 0)
      return (read_err_prog_t){bytes_read, byte_iter};
    byte_iter += bytes_read;
  }

  if (program_iter < program->count)
    return (read_err_prog_t){READ_ERR_EXPECTED_MORE, 0};
  *size_bytes_read = byte_iter;
  return (read_err_prog_t){0};
}
