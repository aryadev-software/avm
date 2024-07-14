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

#include <stdbool.h>
#include <stdio.h>

typedef enum
{
  OP_NOOP = 0,
  OP_HALT,

  // Dealing with data and registers
  OP_PUSH,
  OP_POP,
  OP_PUSH_REGISTER,
  OP_MOV,
  OP_DUP,
  // Boolean operations
  OP_NOT,
  OP_OR,
  OP_AND,
  OP_XOR,
  OP_EQ,

  // Mathematical operations
  OP_PLUS_UNSIGNED,
  OP_MULT_UNSIGNED,
  OP_SUB_UNSIGNED,
  OP_PLUS,
  OP_SUB,
  OP_MULT,

  // Comparison operations
  OP_LT_UNSIGNED,
  OP_LTE_UNSIGNED,
  OP_GT_UNSIGNED,
  OP_GTE_UNSIGNED,
  OP_LT,
  OP_LTE,
  OP_GT,
  OP_GTE,

  // Program control flow
  OP_JUMP_ABS,
  OP_JUMP_IF,

  // Subroutines
  OP_CALL,
  OP_RET,

  // Should not be an opcode
  NUMBER_OF_OPCODES,
} opcode_t;

/**
   @brief An instruction for the virtual machine.  Composed of an opcode and
   zero or more operands.

   @member[opcode] Opcode of instruction
   @member[n] Either the only operand or the number of operands.
   @member[operands] Operands as bytes.
 */
typedef struct
{
  opcode_t opcode;
  word_t n;
  byte_t *operands;
} inst_t;

/**
   @brief A buffer of bytecode.  Used in (de)serialising.
   @member[bytes] Array of bytes.
   @member[cursor] Current index in array.
   @member[size] Size of array.
 */
typedef struct
{
  byte_t *bytes;
  size_t cursor, size;
} bytecode_t;

/**
   @brief Read some number of bytes from bytecode.
   @details Does not convert to host endian.
 */
byte_t *bytecode_read_bytes(bytecode_t *, size_t);

/**
   @brief Read a word from bytecode, converting to host endian.
 */
bool bytecode_read_word(bytecode_t *, word_t *);

#define IS_BYTECODE_DONE(B)   ((B)->cursor >= (B)->size)
#define BYTECODE_REMAINING(B) (WORD_SAFE_SUB((B)->size, (B)->cursor))

#define IS_OPCODE_NULLARY(OP) \
  ((OP) == OP_NOOP || (OP) == OP_HALT || (OP) == OP_RET)

#define IS_OPCODE_UNARY(OP)                                                    \
  ((OP) == OP_POP || (OP) == OP_PUSH_REGISTER || (OP) == OP_DUP ||             \
   (OP) == OP_NOT || (OP) == OP_OR || (OP) == OP_AND || (OP) == OP_XOR ||      \
   (OP) == OP_EQ || (OP) == OP_PLUS_UNSIGNED || (OP) == OP_MULT_UNSIGNED ||    \
   (OP) == OP_SUB_UNSIGNED || (OP) == OP_PLUS || (OP) == OP_SUB ||             \
   (OP) == OP_MULT || (OP) == OP_LT_UNSIGNED || (OP) == OP_LTE_UNSIGNED ||     \
   (OP) == OP_GT_UNSIGNED || (OP) == OP_GTE_UNSIGNED || (OP) == OP_LT ||       \
   (OP) == OP_LTE || (OP) == OP_GT || (OP) == OP_GTE || (OP) == OP_JUMP_ABS || \
   (OP) == OP_CALL)

#define IS_OPCODE_BINARY(OP) ((OP) == OP_MOV || (OP) == OP_JUMP_IF)

#define IS_OPCODE_NARY(OP) ((OP) == OP_PUSH)

/**
   @brief Convert an opcode to a C String.
 */
const char *opcode_as_cstr(opcode_t);

/**
   @brief Print out an instruction to a file.
 */
void inst_print(FILE *, inst_t);

#endif
