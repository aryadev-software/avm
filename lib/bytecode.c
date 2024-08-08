/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * details.

 * You may distribute and modify this code under the terms of the GNU General
 * Public License Version 2, which you should have received a copy of along with
 * this program.  If not, please go to <https://www.gnu.org/licenses/>.

 * Created: 2024-07-25
 * Author: Aryadev Chavali
 * Description: (De)serialising bytecode of programs.
 */

#include <lib/bytecode.h>

#include <assert.h>
#include <string.h>

byte_t *bytecode_read_bytes(bytecode_t *buffer, size_t n)
{
  if (BYTECODE_REMAINING(buffer) < n)
    return NULL;
  byte_t *ptr = buffer->data + buffer->used;
  buffer->used += n;
  return ptr;
}

bool bytecode_write_bytes(bytecode_t *buffer, byte_t *bytes, size_t n)
{
  if (BYTECODE_REMAINING(buffer) < n)
    return false;
  memcpy(buffer->data + buffer->used, bytes, n);
  buffer->used += n;
  return true;
}

bool bytecode_read_word(bytecode_t *buffer, word_t *word_ptr)
{
  if (BYTECODE_REMAINING(buffer) < WORD_SIZE)
    return false;

  byte_t bytes[WORD_SIZE];
  memcpy(bytes, buffer->data + buffer->used, WORD_SIZE);
  convert_bytes_le(bytes, ARR_SIZE(bytes));
  memcpy(word_ptr, bytes, WORD_SIZE);

  buffer->used += WORD_SIZE;
  return true;
}

bool bytecode_write_word(bytecode_t *buffer, word_t word)
{
  if (BYTECODE_REMAINING(buffer) < WORD_SIZE)
    return false;

  byte_t bytes[WORD_SIZE];
  memcpy(bytes, &word, WORD_SIZE);
  convert_bytes_le(bytes, WORD_SIZE);
  memcpy(bytes, buffer->data + buffer->used, WORD_SIZE);

  buffer->used += WORD_SIZE;
  return true;
}

bool bytecode_read_inst(bytecode_t *buffer, inst_t *inst)
{
  static_assert(NUMBER_OF_OPCODES == 30, "bytecode_read_inst is out of date.");
  if (BYTECODE_REMAINING(buffer) == 0)
    return false;
  inst->opcode = buffer->data[buffer->used];
  if (inst->opcode < OP_NOOP || inst->opcode >= NUMBER_OF_OPCODES)
    return false;
  ++buffer->used;
  if (IS_OPCODE_NULLARY(inst->opcode))
    // We are done
    return true;

  bool success = bytecode_read_word(buffer, &inst->n);
  if (!success || IS_OPCODE_UNARY(inst->opcode))
    return success;

  if (IS_OPCODE_BINARY(inst->opcode))
  {
    if (BYTECODE_REMAINING(buffer) < WORD_SIZE)
      return false;
    // Convert the next WORD_SIZE bytes into a host endian word for use.
    convert_bytes_le(buffer->data + buffer->used, WORD_SIZE);
  }
  else if (IS_OPCODE_NARY(inst->opcode))
  {
    // Check we have enough space
    if (BYTECODE_REMAINING(buffer) < inst->n)
      return false;
  }
  inst->operands = buffer->data + buffer->used;
  buffer->used += inst->n;
  return true;
}

bool bytecode_write_inst(bytecode_t *buffer, inst_t inst)
{
  if (BYTECODE_REMAINING(buffer) < 1)
    return false;
  buffer->data[buffer->used++] = inst.opcode;
  if (IS_OPCODE_UNARY(inst.opcode))
  {
  }
  else if (IS_OPCODE_UNARY(inst.opcode))
  {
    return bytecode_write_word(buffer, inst.n);
  }
  else if (IS_OPCODE_BINARY(inst.opcode))
  {
    bool succ = bytecode_write_word(buffer, inst.n);
    if (!succ)
      return succ;
    convert_bytes_le(inst.operands, WORD_SIZE);
    memcpy(buffer->data + buffer->used, inst.operands, WORD_SIZE);
    buffer->used += WORD_SIZE;
  }
  else if (IS_OPCODE_NARY(inst.opcode))
  {
    bool succ = bytecode_write_word(buffer, inst.n);
    if (!succ)
      return succ;
    memcpy(buffer->data + buffer->used, inst.operands, inst.n);
    buffer->used += inst.n;
  }
  return true;
}

bool bytecode_read_prog_header(bytecode_t *bytecode, prog_header_t *header)
{
  if (BYTECODE_REMAINING(bytecode) < sizeof(prog_header_t))
    return false;
  return bytecode_read_word(bytecode, &header->start) &&
         bytecode_read_word(bytecode, &header->count);
}

bool bytecode_write_prog_header(bytecode_t *bytecode, prog_header_t header)
{
  if (BYTECODE_REMAINING(bytecode) < sizeof(header))
    return false;
  return bytecode_write_word(bytecode, header.start) &&
         bytecode_write_word(bytecode, header.count);
}
