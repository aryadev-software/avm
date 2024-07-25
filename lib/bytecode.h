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

#define BYTECODE_REMAINING(B) (WORD_SAFE_SUB((B)->size, (B)->cursor))
#define IS_BYTECODE_DONE(B)   ((B)->cursor >= (B)->size)

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

#endif
