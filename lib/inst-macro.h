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

#include "./inst.h"

#define INST_NOOP ((inst_t){0})
#define INST_HALT ((inst_t){.opcode = OP_HALT})

#define INST_PUSH(TYPE, OP) \
  ((inst_t){.opcode = OP_PUSH_##TYPE, .operand = D##TYPE(OP)})

#define INST_MOV(TYPE, OP) \
  ((inst_t){.opcode = OP_MOV_##TYPE, .operand = DWORD(OP)})

#define INST_POP(TYPE) ((inst_t){.opcode = OP_POP_##TYPE})

#define INST_PUSH_REG(TYPE, REG) \
  ((inst_t){.opcode = OP_PUSH_REGISTER_##TYPE, .operand = D##TYPE(REG)})

#define INST_DUP(TYPE, OP) \
  ((inst_t){.opcode = OP_DUP_##TYPE, .operand = DWORD(OP)})

#define INST_MALLOC(TYPE, OP) \
  ((inst_t){.opcode = OP_MALLOC_##TYPE, .operand = DWORD(OP)})
#define INST_MALLOC_STACK(TYPE) ((inst_t){.opcode = OP_MALLOC_STACK_##TYPE})
#define INST_MSET(TYPE, OP) \
  ((inst_t){.opcode = OP_MSET_##TYPE, .operand = DWORD(OP)})
#define INST_MSET_STACK(TYPE) ((inst_t){.opcode = OP_MSET_STACK_##TYPE})
#define INST_MGET(TYPE, OP) \
  ((inst_t){.opcode = OP_MGET_##TYPE, .operand = DWORD(OP)})
#define INST_MGET_STACK(TYPE) ((inst_t){.opcode = OP_MGET_STACK_##TYPE})
#define INST_MDELETE          ((inst_t){.opcode = OP_MDELETE})
#define INST_MSIZE            ((inst_t){.opcode = OP_MSIZE})

#define INST_NOT(TYPE)  ((inst_t){.opcode = OP_NOT_##TYPE})
#define INST_OR(TYPE)   ((inst_t){.opcode = OP_OR_##TYPE})
#define INST_AND(TYPE)  ((inst_t){.opcode = OP_AND_##TYPE})
#define INST_XOR(TYPE)  ((inst_t){.opcode = OP_XOR_##TYPE})
#define INST_EQ(TYPE)   ((inst_t){.opcode = OP_EQ_##TYPE})
#define INST_LT(TYPE)   ((inst_t){.opcode = OP_LT_##TYPE})
#define INST_LTE(TYPE)  ((inst_t){.opcode = OP_LTE_##TYPE})
#define INST_GT(TYPE)   ((inst_t){.opcode = OP_GT_##TYPE})
#define INST_GTE(TYPE)  ((inst_t){.opcode = OP_GTE_##TYPE})
#define INST_PLUS(TYPE) ((inst_t){.opcode = OP_PLUS_##TYPE})
#define INST_SUB(TYPE)  ((inst_t){.opcode = OP_SUB_##TYPE})
#define INST_MULT(TYPE) ((inst_t){.opcode = OP_MULT_##TYPE})

#define INST_JUMP_ABS(OP) \
  ((inst_t){.opcode = OP_JUMP_ABS, .operand = DWORD(OP)})
#define INST_JUMP_STACK ((inst_t){.opcode = OP_JUMP_STACK})
#define INST_JUMP_IF(TYPE, OP) \
  ((inst_t){.opcode = OP_JUMP_IF_##TYPE, .operand = DWORD(OP)})
#define INST_CALL(OP)   ((inst_t){.opcode = OP_CALL, .operand = DWORD(OP)})
#define INST_CALL_STACK ((inst_t){.opcode = OP_CALL_STACK})
#define INST_RET        ((inst_t){.opcode = OP_RET})

#define INST_PRINT(TYPE) ((inst_t){.opcode = OP_PRINT_##TYPE})

#endif
