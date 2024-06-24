/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Dynamically sized byte array
 */

#ifndef DARR_H
#define DARR_H

#include <stdio.h>

#include "./base.h"

/**
   @brief A dynamically sized buffer of bytes.

   @prop[data] Buffer of bytes (may be reallocated)

   @prop[used] Number of bytes currently used

   @prop[available] Number of bytes currently allocated
 */
typedef struct
{
  byte_t *data;
  size_t used, available;
} darr_t;

/* Some useful constants for dynamic array work. */
#define DARR_DEFAULT_SIZE 8
#define DARR_REALLOC_MULT 2

/**
   @brief Get the `IND`th item of type `TYPE` in `DARR_DATA`

   @details Cast `DARR_DATA` to `TYPE`, taking the `IND`th item.
   NOTE: This is unsafe as bound checks are not done i.e. if
   `DARR_DATA` has at least space for `IND` * sizeof(`TYPE`) items.
   It is presumed the caller will check themselves.

   @param[TYPE] Type to cast internal byte array
   @param[DARR_DATA] Byte array of darr
   @param[IND] Index of item

   @return Item of TYPE
 */
#define DARR_AT(TYPE, DARR_DATA, IND) ((TYPE *)(DARR_DATA))[(IND)]

/**
   @brief Initialise a dynamic array `darr` with n bytes of space.

   @details All properties of `darr` are initialised.  `darr`.used is
   set to 0, `darr`.available is set to `n` and `darr`.data is set to
   a pointer of `n` bytes.  NOTE: If `n` = 0 then it is set to
   DARR_DEFAULT_SIZE

   @param[darr] Pointer to darr_t object to initialise
   @param[n] Number of bytes to allocate.  If equal to 0 then
   considered treated as DARR_DEFAULT_SIZE
 */
void darr_init(darr_t *darr, size_t n);

/**
   @brief Ensure a dynamic array has at least n bytes of space free.

   @details If `darr` has n or more bytes free, nothing occurs.
   Otherwise, the byte array in `darr` is reallocated such that it has
   at least `n` bytes of free space.  NOTE: `darr` has at least `n`
   bytes free if and only if `darr`.used + `n` <= `darr`.available

   @param[darr] Dynamic array to check
   @param[n] Number of bytes
 */
void darr_ensure_capacity(darr_t *darr, size_t n);

/**
   @brief Append a byte to a dynamic array.

   @details Append a byte to the end of the byte buffer in a dyamic
   array.  If the dynamic array doesn't have enough free space to fit
   the byte, it will reallocate to ensure it can fit it in via
   darr_ensure_capacity().

   @param[darr] Dynamic arrary to append to
   @param[b] Byte to append
 */
void darr_append_byte(darr_t *darr, byte_t b);

/**
   @brief Append an array of n bytes to a dynamic array.

   @details Append an array of bytes to the end of a byte buffer.  If
   the dynamic array doesn't have enough free space to fit all n bytes
   it will reallocate to ensure it can fit it in via
   darr_ensure_capacity().

   @param[darr] Dynamic array to append to
   @param[b] Array of bytes to append
   @param[n] Size of array of bytes
 */
void darr_append_bytes(darr_t *darr, byte_t *b, size_t n);

/**
   @brief Get the nth byte of a dynamic array

   @details Get the nth byte of the dynamic array.  0 based.  NOTE: If
   the dynamic array has less than n bytes used, it will return 0 as a
   default value, so this is a safe alternative to DARR_AT().

   @param[darr] Dynamic array to index
   @param[n] Index to get byte at

   @return Byte at the nth position, or 0 if n is an invalid index
 */
byte_t *darr_at(darr_t *darr, size_t n);

/**
   @brief Write the bytes of a dynamic array to a file pointer

   @details Given a dynamic array and a file pointer, write the
   internal buffer of bytes to the file pointer.  NOTE: The file
   pointer is assumed to be open and suitable for writing.

   @param[darr] Dynamic array to write
   @param[fp] File pointer to write on
 */
void darr_write_file(darr_t *darr, FILE *fp);

/**
   @brief Read a file pointer in its entirety into a dynamic array

   @details Read a file pointer as a buffer of bytes then return that
   buffer wrapped in a darr_t structure.  NOTE: the file pointer is
   assumed to be open and suitable for reading.

   @param[fp]: File pointer to read

   @return Dynamic array structure with available set to the size of
   the `buffer` read and `data` set to the buffer of bytes.
 */
darr_t darr_read_file(FILE *fp);

#endif
