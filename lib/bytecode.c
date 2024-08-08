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

#include "lib/inst.h"
#include <lib/base.h>
#include <lib/bytecode.h>

#include <assert.h>
#include <string.h>

size_t bytecode_inst_size(inst_t inst)
{
  if (IS_OPCODE_NULLARY(inst.opcode))
    return 1;
  else if (IS_OPCODE_UNARY(inst.opcode))
    return 1 + WORD_SIZE;
  else if (IS_OPCODE_BINARY(inst.opcode))
    return 1 + (2 * WORD_SIZE);
  else if (IS_OPCODE_NARY(inst.opcode))
    return 1 + WORD_SIZE + inst.n;
  else
    return 0;
}

size_t bytecode_prog_size(prog_t prog)
{
  size_t size = sizeof(prog.header);
  for (size_t i = 0; i < prog.header.count; ++i)
  {
    size_t inst_size = bytecode_inst_size(prog.instructions[i]);
    if (inst_size == 0)
      return 0;
    size += inst_size;
  }
  return size;
}

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

  memcpy(buffer->data + buffer->used, &word, WORD_SIZE);
  convert_bytes_le(buffer->data + buffer->used, WORD_SIZE);

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

  inst->operands = buffer->data + buffer->used;

  if (IS_OPCODE_BINARY(inst->opcode))
  {
    if (BYTECODE_REMAINING(buffer) < WORD_SIZE)
      return false;
    // Convert the next WORD_SIZE bytes into a host endian word for use.
    convert_bytes_le(buffer->data + buffer->used, WORD_SIZE);
    buffer->used += WORD_SIZE;
  }
  else if (IS_OPCODE_NARY(inst->opcode))
  {
    // Check we have enough space
    if (BYTECODE_REMAINING(buffer) < inst->n)
      return false;
    buffer->used += inst->n;
  }
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

i64 bytecode_read_n_ops(bytecode_t *bytecode)
{
  static_assert(NUMBER_OF_OPCODES == 30, "bytecode_read_n_ops is out of date.");
  /*
    1) Find an instruction opcode
    2) Figure out if it takes any arguments (that cannot be stored in inst.n)
    3) Count the arguments
   */
  if (BYTECODE_REMAINING(bytecode) == 0)
    return -1;
  i64 ops = 0;
  for (word_t i = bytecode->used; i < bytecode->available;)
  {
    int opcode = bytecode->data[i];
    if (!IS_OPCODE(opcode))
      return -1; // Invalid
    else if (IS_OPCODE_NULLARY(opcode))
      ++i;
    else if (IS_OPCODE_UNARY(opcode))
    {
      // increment i by a word
      i += WORD_SIZE;
    }
    else if (IS_OPCODE_BINARY(opcode))
    {
      ops += WORD_SIZE;
      i += WORD_SIZE;
    }
    else if (IS_OPCODE_NARY(opcode))
    {
      // Read the word in bytecode
      word_t w = 0;
      byte_t bytes[WORD_SIZE];
      memcpy(bytes, bytecode->data + i + 1, WORD_SIZE);
      convert_bytes_le(bytes, WORD_SIZE);
      memcpy(&w, bytes, WORD_SIZE);

      i += WORD_SIZE + w;
      ops += w;
    }
  }

  return ops;
}
