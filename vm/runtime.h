/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Virtual machine implementation
 */

#ifndef RUNTIME_H
#define RUNTIME_H

#include <vm/struct.h>

typedef enum
{
  ERR_OK = 0,
  ERR_STACK_UNDERFLOW,
  ERR_STACK_OVERFLOW,
  ERR_CALL_STACK_UNDERFLOW,
  ERR_CALL_STACK_OVERFLOW,
  ERR_INVALID_OPCODE,
  ERR_INVALID_REGISTER_BYTE,
  ERR_INVALID_REGISTER_SHORT,
  ERR_INVALID_REGISTER_HWORD,
  ERR_INVALID_REGISTER_WORD,
  ERR_INVALID_PROGRAM_ADDRESS,
  ERR_INVALID_PAGE_ADDRESS,
  ERR_OUT_OF_BOUNDS,
  ERR_END_OF_PROGRAM,
} err_t;

const char *err_as_cstr(err_t);

err_t vm_execute(vm_t *);
err_t vm_execute_all(vm_t *);

err_t vm_jump(vm_t *, word_t);

err_t vm_push(vm_t *, word_t, byte_t *);
err_t vm_pop(vm_t *, word_t, byte_t **);
err_t vm_push_register(vm_t *, word_t, word_t);
err_t vm_mov(vm_t *, word_t, word_t);
err_t vm_dup(vm_t *, word_t);

err_t vm_not(vm_t *, word_t);
err_t vm_or(vm_t *, word_t);
err_t vm_and(vm_t *, word_t);
err_t vm_xor(vm_t *, word_t);
err_t vm_eq(vm_t *, word_t);
err_t vm_lt(vm_t *, word_t);
err_t vm_lte(vm_t *, word_t);
err_t vm_gt(vm_t *, word_t);
err_t vm_gte(vm_t *, word_t);
err_t vm_lt_unsigned(vm_t *, word_t);
err_t vm_lte_unsigned(vm_t *, word_t);
err_t vm_gt_unsigned(vm_t *, word_t);
err_t vm_gte_unsigned(vm_t *, word_t);
err_t vm_plus(vm_t *, word_t);
err_t vm_plus_unsigned(vm_t *, word_t);
err_t vm_sub(vm_t *, word_t);
err_t vm_sub_unsigned(vm_t *, word_t);
err_t vm_mult(vm_t *, word_t);
err_t vm_mult_unsigned(vm_t *, word_t);

#define IS_OPCODE_VM_UNARY(OP)                                               \
  ((OP) == OP_DUP || (OP) == OP_NOT || (OP) == OP_OR || (OP) == OP_AND ||    \
   (OP) == OP_XOR || (OP) == OP_EQ || (OP) == OP_LT || (OP) == OP_LTE ||     \
   (OP) == OP_GT || (OP) == OP_GTE || (OP) == OP_LT_UNSIGNED ||              \
   (OP) == OP_LTE_UNSIGNED || (OP) == OP_GT_UNSIGNED ||                      \
   (OP) == OP_GTE_UNSIGNED || (OP) == OP_PLUS || (OP) == OP_PLUS_UNSIGNED || \
   (OP) == OP_SUB || (OP) == OP_SUB_UNSIGNED || (OP) == OP_MULT ||           \
   (OP) == OP_MULT_UNSIGNED)

typedef err_t (*unary_word_routine)(vm_t *, word_t);
unary_word_routine UNARY_WORD_ROUTINES[] = {
    [OP_DUP]           = vm_dup,
    [OP_NOT]           = vm_not,
    [OP_OR]            = vm_or,
    [OP_AND]           = vm_and,
    [OP_XOR]           = vm_xor,
    [OP_EQ]            = vm_eq,
    [OP_LT]            = vm_lt,
    [OP_LTE]           = vm_lte,
    [OP_GT]            = vm_gt,
    [OP_GTE]           = vm_gte,
    [OP_LT_UNSIGNED]   = vm_lt_unsigned,
    [OP_LTE_UNSIGNED]  = vm_lte_unsigned,
    [OP_GT_UNSIGNED]   = vm_gt_unsigned,
    [OP_GTE_UNSIGNED]  = vm_gte_unsigned,
    [OP_PLUS]          = vm_plus,
    [OP_PLUS_UNSIGNED] = vm_plus_unsigned,
    [OP_SUB]           = vm_sub,
    [OP_SUB_UNSIGNED]  = vm_sub_unsigned,
    [OP_MULT]          = vm_mult,
    [OP_MULT_UNSIGNED] = vm_mult_unsigned,
};

#endif
