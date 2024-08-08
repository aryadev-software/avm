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

#ifndef BYTECODE_H
#define BYTECODE_H

#include <lib/base.h>
#include <lib/darr.h>
#include <lib/inst.h>

// Bytecode buffer is simply a dynamic array
typedef darr_t bytecode_t;

#define BYTECODE_REMAINING(B) (WORD_SAFE_SUB((B)->available, (B)->used))
#define IS_BYTECODE_DONE(B)   ((B)->used >= (B)->available)

/**
   @brief Compute the size of an instruction in bytecode.
   @param[inst_t] Instruction to find size for.
   @return[size_t] Size of instruction in bytes.  May be zero if ill formed.
 */
size_t bytecode_inst_size(inst_t);

/**
   @brief Compute the size of a program in bytecode.
   @param[prog_t] Program to find size for.
   @return[size_t] Size of program in bytes.  May be zero if ill formed.
 */
size_t bytecode_prog_size(prog_t);

/**
   @brief Read some number of bytes from bytecode.
   @details Does not convert to host endian.
   @return[byte_t*] Pointer to start of bytes, NULL if not right size
 */
byte_t *bytecode_read_bytes(bytecode_t *, size_t);

/**
   @brief Write some number of bytes to bytecode.
   @details Does not convert to Little Endian
   @return[bool] Successful write to bytecode
 */
bool bytecode_write_bytes(bytecode_t *, byte_t *, size_t);

/**
   @brief Read a word from bytecode, converting to host endian.
   @return[bool] Success in reading a word.
 */
bool bytecode_read_word(bytecode_t *, word_t *);

/**
   @brief Write a word to bytecode in Little Endian.
   @return[bool] Success in writing word.
 */
bool bytecode_write_word(bytecode_t *, word_t);

/**
   @brief Read an instruction from bytecode.
   @return[bool] Success in reading the instruction.
 */
bool bytecode_read_inst(bytecode_t *, inst_t *);

/**
   @brief Write an instruction to bytecode.
   @return[bool] Success in writing instruction.
 */
bool bytecode_write_inst(bytecode_t *, inst_t);

/**
   @brief Read a program header from bytecode.
   @return[bool] Success in reading header.
 */
bool bytecode_read_prog_header(bytecode_t *, prog_header_t *);

/**
   @brief Write a program header to bytecode.
   @return[bool] Success in writing header.
 */
bool bytecode_write_prog_header(bytecode_t *, prog_header_t);

/**
   @brief Find the number of operands that must be stored as bytes.
   @return[size_t] Number of operands.
 */
i64 bytecode_read_n_ops(bytecode_t *);

#endif
