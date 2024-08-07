/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Instructions and opcodes
 */

#ifndef INST_H
#define INST_H

#include <lib/base.h>
#include <stdio.h>

#define UNSIGNED_OPCODE_IS_TYPE(OPCODE, OP_TYPE) \
  (((OPCODE) >= OP_TYPE##_BYTE) && ((OPCODE) <= OP_TYPE##_WORD))

#define SIGNED_OPCODE_IS_TYPE(OPCODE, OP_TYPE) \
  (((OPCODE) >= OP_TYPE##_BYTE) && ((OPCODE) <= OP_TYPE##_SWORD))

#define OPCODE_DATA_TYPE(OPCODE, OP_TYPE) (OPCODE - OP_TYPE##_BYTE)

typedef enum
{
  OP_NOOP = 0,
  OP_HALT,

  // Dealing with data and registers
  OP_PUSH_BYTE,
  OP_PUSH_SHORT,
  OP_PUSH_HWORD,
  OP_PUSH_WORD,

  OP_POP_BYTE,
  OP_POP_SHORT,
  OP_POP_HWORD,
  OP_POP_WORD,

  OP_PUSH_REGISTER_BYTE,
  OP_PUSH_REGISTER_SHORT,
  OP_PUSH_REGISTER_HWORD,
  OP_PUSH_REGISTER_WORD,

  OP_MOV_BYTE,
  OP_MOV_SHORT,
  OP_MOV_HWORD,
  OP_MOV_WORD,

  OP_DUP_BYTE,
  OP_DUP_SHORT,
  OP_DUP_HWORD,
  OP_DUP_WORD,

  // Dealing with the heap
  OP_MALLOC_BYTE,
  OP_MALLOC_SHORT,
  OP_MALLOC_HWORD,
  OP_MALLOC_WORD,

  OP_MSET_BYTE,
  OP_MSET_SHORT,
  OP_MSET_HWORD,
  OP_MSET_WORD,

  OP_MGET_BYTE,
  OP_MGET_SHORT,
  OP_MGET_HWORD,
  OP_MGET_WORD,

  OP_MDELETE,
  OP_MSIZE,

  // Boolean operations
  OP_NOT_BYTE,
  OP_NOT_SHORT,
  OP_NOT_HWORD,
  OP_NOT_WORD,

  OP_OR_BYTE,
  OP_OR_SHORT,
  OP_OR_HWORD,
  OP_OR_WORD,

  OP_AND_BYTE,
  OP_AND_SHORT,
  OP_AND_HWORD,
  OP_AND_WORD,

  OP_XOR_BYTE,
  OP_XOR_SHORT,
  OP_XOR_HWORD,
  OP_XOR_WORD,

  OP_EQ_BYTE,
  OP_EQ_SHORT,
  OP_EQ_HWORD,
  OP_EQ_WORD,

  // Mathematical operations
  OP_PLUS_BYTE,
  OP_PLUS_SHORT,
  OP_PLUS_HWORD,
  OP_PLUS_WORD,

  OP_SUB_BYTE,
  OP_SUB_SHORT,
  OP_SUB_HWORD,
  OP_SUB_WORD,

  OP_MULT_BYTE,
  OP_MULT_SHORT,
  OP_MULT_HWORD,
  OP_MULT_WORD,

  // Comparison operations
  OP_LT_BYTE,
  OP_LT_SBYTE,
  OP_LT_SHORT,
  OP_LT_SSHORT,
  OP_LT_HWORD,
  OP_LT_SHWORD,
  OP_LT_WORD,
  OP_LT_SWORD,

  OP_LTE_BYTE,
  OP_LTE_SBYTE,
  OP_LTE_SHORT,
  OP_LTE_SSHORT,
  OP_LTE_HWORD,
  OP_LTE_SHWORD,
  OP_LTE_WORD,
  OP_LTE_SWORD,

  OP_GT_BYTE,
  OP_GT_SBYTE,
  OP_GT_SHORT,
  OP_GT_SSHORT,
  OP_GT_HWORD,
  OP_GT_SHWORD,
  OP_GT_WORD,
  OP_GT_SWORD,

  OP_GTE_BYTE,
  OP_GTE_SBYTE,
  OP_GTE_SHORT,
  OP_GTE_SSHORT,
  OP_GTE_HWORD,
  OP_GTE_SHWORD,
  OP_GTE_WORD,
  OP_GTE_SWORD,

  // Simple I/O
  OP_PRINT_BYTE,
  OP_PRINT_SBYTE,
  OP_PRINT_SHORT,
  OP_PRINT_SSHORT,
  OP_PRINT_HWORD,
  OP_PRINT_SHWORD,
  OP_PRINT_WORD,
  OP_PRINT_SWORD,

  // Program control flow
  OP_JUMP_ABS,
  OP_JUMP_IF_BYTE,
  OP_JUMP_IF_SHORT,
  OP_JUMP_IF_HWORD,
  OP_JUMP_IF_WORD,

  // Subroutines
  OP_CALL,
  OP_RET,

  // Should not be an opcode
  NUMBER_OF_OPCODES,
} opcode_t;

size_t opcode_bytecode_size(opcode_t);
const char *opcode_as_cstr(opcode_t);

typedef struct
{
  opcode_t opcode;
  data_t operand;
} inst_t;

/**
   @brief Serialise an instruction into a byte buffer

   @details Given an instruction and a suitably sized byte buffer, write the
   bytecode for the instruction into the buffer.  NOTE: This function does NOT
   check the bounds of `bytes` i.e. we assume the caller has created a suitably
   sized buffer.

   @param[inst] Instruction to serialise
   @param[bytes] Buffer to write on

   @return[size_t] Number of bytes written to `bytes`.
 */
size_t inst_write_bytecode(inst_t inst, byte_t *bytes);

typedef enum
{
  READ_ERR_INVALID_OPCODE = -1,
  READ_ERR_OPERAND_NO_FIT = -2,
  READ_ERR_EXPECTED_MORE  = -3,
  READ_ERR_END            = -4
} read_err_t;

/**
   @brief Deserialise an instruction from a bytecode buffer

   @details Given a buffer of bytes, deserialise an instruction, storing the
   result in the pointer given.  The number of bytes read in the buffer is
   returned, which should be opcode_bytecode_size().  NOTE: If bytes is not
   suitably sized for the instruction expected or it is not well formed i.e. not
   the right schema then a negative number is returned.

   @param[inst] Pointer to instruction which will store result
   @param[bytes] Bytecode buffer to deserialise
   @param[size_bytes] Number of bytes in buffer

   @return[int] Number of bytes read.  If negative then an error occurred in
   deserialisation (either buffer was not suitably sized or instruction was not
   well formed) so any result must be considered invalid.
 */
int inst_read_bytecode(inst_t *inst, byte_t *bytes, size_t size_bytes);

void inst_print(inst_t, FILE *);

typedef struct
{
  word_t start_address;
  word_t count;
  inst_t *instructions;
} prog_t;

#define PROG_HEADER_SIZE (WORD_SIZE * 2)

size_t prog_bytecode_size(prog_t);

size_t prog_write_bytecode(prog_t program, byte_t *bytes, size_t size_bytes);

size_t prog_read_header(prog_t *program, byte_t *bytes, size_t size_bytes);

typedef struct
{
  read_err_t type;
  size_t index;
} read_err_prog_t;

read_err_prog_t prog_read_instructions(prog_t *program, size_t *size_bytes_read,
                                       byte_t *bytes, size_t size_bytes);

#endif
