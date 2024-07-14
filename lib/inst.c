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
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

byte_t *bytecode_read_bytes(bytecode_t *buffer, size_t n)
{
  if (BYTECODE_REMAINING(buffer) < n)
    return NULL;
  byte_t *ptr = buffer->bytes + buffer->cursor;
  buffer->cursor += n;
  return ptr;
}

bool bytecode_read_word(bytecode_t *buffer, word_t *word_ptr)
{
  if (BYTECODE_REMAINING(buffer) < WORD_SIZE)
    return false;

  byte_t bytes[WORD_SIZE];
  memcpy(bytes, buffer->bytes + buffer->cursor, WORD_SIZE);
  convert_bytes_le(bytes, ARR_SIZE(bytes));
  memcpy(word_ptr, bytes, WORD_SIZE);

  buffer->cursor += WORD_SIZE;
  return true;
}

bool bytecode_read_inst(bytecode_t *buffer, inst_t *inst)
{
  static_assert(NUMBER_OF_OPCODES == 30, "bytecode_read_inst is out of date.");
  if (BYTECODE_REMAINING(buffer) == 0)
    return false;
  inst->opcode = buffer->bytes[buffer->cursor];
  if (inst->opcode < OP_NOOP || inst->opcode >= NUMBER_OF_OPCODES)
    return false;
  ++buffer->cursor;
  if (IS_OPCODE_NULLARY(inst->opcode))
    // We are done
    return true;

  bool success = bytecode_read_word(buffer, &inst->n);
  if (!success || IS_OPCODE_UNARY(inst->opcode))
    return success;

  // Check we have enough space
  if (BYTECODE_REMAINING(buffer) < inst->n)
    return false;

  if (IS_OPCODE_BINARY(inst->opcode))
  {
    // We need to convert the operand bytes, which is a word, into host endian
    convert_bytes_le(buffer->bytes + buffer->cursor, WORD_SIZE);
  }
  else if (IS_OPCODE_NARY(inst->opcode))
  {
    // Ordering is important, so don't convert.
  }
  inst->operands = buffer->bytes + buffer->cursor;
  buffer->cursor += inst->n;
  return true;
}

const char *opcode_as_cstr(opcode_t code)
{
  switch (code)
  {
  case OP_NOOP:
    return "NOOP";
  case OP_HALT:
    return "HALT";
  case OP_PUSH:
    return "PUSH";
  case OP_POP:
    return "POP";
  case OP_PUSH_REGISTER:
    return "PUSH_REGISTER";
  case OP_MOV:
    return "MOV";
  case OP_DUP:
    return "DUP";
  case OP_NOT:
    return "NOT";
  case OP_OR:
    return "OR";
  case OP_AND:
    return "AND";
  case OP_XOR:
    return "XOR";
  case OP_EQ:
    return "EQ";
  case OP_PLUS_UNSIGNED:
    return "PLUS_UNSIGNED";
  case OP_MULT_UNSIGNED:
    return "MULT_UNSIGNED";
  case OP_SUB_UNSIGNED:
    return "SUB_UNSIGNED";
  case OP_PLUS:
    return "PLUS";
  case OP_SUB:
    return "SUB";
  case OP_MULT:
    return "MULT";
  case OP_LT_UNSIGNED:
    return "LT_UNSIGNED";
  case OP_LTE_UNSIGNED:
    return "LTE_UNSIGNED";
  case OP_GT_UNSIGNED:
    return "GT_UNSIGNED";
  case OP_GTE_UNSIGNED:
    return "GTE_UNSIGNED";
  case OP_LT:
    return "LT";
  case OP_LTE:
    return "LTE";
  case OP_GT:
    return "GT";
  case OP_GTE:
    return "GTE";
  case OP_JUMP_ABS:
    return "JUMP_ABS";
  case OP_JUMP_IF:
    return "JUMP_IF";
  case OP_CALL:
    return "CALL";
  case OP_RET:
    return "RET";
  case NUMBER_OF_OPCODES:
    return "";
  }
  return "";
}

void inst_print(FILE *fp, inst_t instruction)
{
  fprintf(fp, "%s(", opcode_as_cstr(instruction.opcode));
  fprintf(fp, "%" PRIu64, instruction.n);
  if (IS_OPCODE_BINARY(instruction.opcode))
  {
    // Interpret operands as a word
    word_t word = 0;
    memcpy(&word, instruction.operands, WORD_SIZE);
    fprintf(fp, ", %" PRIu64, word);
  }
  else if (IS_OPCODE_NARY(instruction.opcode))
  {
    // Operands as bytes
    fprintf(fp, ", ");
    print_byte_array(fp, instruction.operands, instruction.n);
  }
  fprintf(fp, ")");
}
