/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2024-04-28
 * Author: Aryadev Chavali
 * Description: Macros for creating inst_t instances quickly
 */

#ifndef INST_MACRO_H
#define INST_MACRO_H

#define INST_NOOP ((inst_t){0})
#define INST_HALT ((inst_t){.opcode = OP_HALT, .n = 0, .operands = NULL})

#define INST_PUSH(N, OPS) ((inst_t){.opcode = OP_PUSH, .n = N, .operands = OPS})
#define INST_PUSH_REGISTER(N, OPS) \
  ((inst_t){.opcode = OP_PUSH_REGISTER, .n = N, .operands = OPS})
#define INST_JUMP_IF(N, OPS) \
  ((inst_t){.opcode = OP_JUMP_IF, .n = N, .operands = OPS})
#define INST_MOV(N, OPS) ((inst_t){.opcode = OP_MOV, .n = N, .operands = OPS})

#define INST_POP(N) ((inst_t){.opcode = OP_POP, .n = N, .operands = NULL})
#define INST_DUP(N) ((inst_t){.opcode = OP_DUP, .n = N, .operands = NULL})
#define INST_NOT(N) ((inst_t){.opcode = OP_NOT, .n = N, .operands = NULL})
#define INST_OR(N)  ((inst_t){.opcode = OP_OR, .n = N, .operands = NULL})
#define INST_AND(N) ((inst_t){.opcode = OP_AND, .n = N, .operands = NULL})
#define INST_XOR(N) ((inst_t){.opcode = OP_XOR, .n = N, .operands = NULL})
#define INST_EQ(N)  ((inst_t){.opcode = OP_EQ, .n = N, .operands = NULL})
#define INST_PLUS_UNSIGNED(N) \
  ((inst_t){.opcode = OP_PLUS_UNSIGNED, .n = N, .operands = NULL})
#define INST_MULT_UNSIGNED(N) \
  ((inst_t){.opcode = OP_MULT_UNSIGNED, .n = N, .operands = NULL})
#define INST_SUB_UNSIGNED(N) \
  ((inst_t){.opcode = OP_SUB_UNSIGNED, .n = N, .operands = NULL})
#define INST_PLUS(N) ((inst_t){.opcode = OP_PLUS, .n = N, .operands = NULL})
#define INST_SUB(N)  ((inst_t){.opcode = OP_SUB, .n = N, .operands = NULL})
#define INST_MULT(N) ((inst_t){.opcode = OP_MULT, .n = N, .operands = NULL})
#define INST_LT_UNSIGNED(N) \
  ((inst_t){.opcode = OP_LT_UNSIGNED, .n = N, .operands = NULL})
#define INST_LTE_UNSIGNED(N) \
  ((inst_t){.opcode = OP_LTE_UNSIGNED, .n = N, .operands = NULL})
#define INST_GT_UNSIGNED(N) \
  ((inst_t){.opcode = OP_GT_UNSIGNED, .n = N, .operands = NULL})
#define INST_GTE_UNSIGNED(N) \
  ((inst_t){.opcode = OP_GTE_UNSIGNED, .n = N, .operands = NULL})
#define INST_LT(N)  ((inst_t){.opcode = OP_LT, .n = N, .operands = NULL})
#define INST_LTE(N) ((inst_t){.opcode = OP_LTE, .n = N, .operands = NULL})
#define INST_GT(N)  ((inst_t){.opcode = OP_GT, .n = N, .operands = NULL})
#define INST_GTE(N) ((inst_t){.opcode = OP_GTE, .n = N, .operands = NULL})
#define INST_JUMP_ABS(N) \
  ((inst_t){.opcode = OP_JUMP_ABS, .n = N, .operands = NULL})
#define INST_CALL(N) ((inst_t){.opcode = OP_CALL, .n = N, .operands = NULL})
#define INST_RET(N)  ((inst_t){.opcode = OP_RET, .n = N, .operands = NULL})

#endif
