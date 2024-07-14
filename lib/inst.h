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

#endif
