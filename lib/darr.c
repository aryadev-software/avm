/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Dynamically sized byte array
 */

#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "./darr.h"

void darr_init(darr_t *darr, size_t size)
{
  if (size == 0)
    size = DARR_DEFAULT_SIZE;
  *darr = (darr_t){
      .data      = calloc(size, 1),
      .used      = 0,
      .available = size,
  };
}

void darr_ensure_capacity(darr_t *darr, size_t requested)
{
  if (darr->used + requested >= darr->available)
  {
    darr->available =
        MAX(darr->used + requested, darr->available * DARR_REALLOC_MULT);
    darr->data = realloc(darr->data, darr->available);
    memset(darr->data + darr->used, 0, darr->available - darr->used);
  }
}

void darr_append_byte(darr_t *darr, byte_t byte_t)
{
  darr_ensure_capacity(darr, 1);
  darr->data[darr->used++] = byte_t;
}

void darr_append_bytes(darr_t *darr, byte_t *bytes, size_t n)
{
  darr_ensure_capacity(darr, n);
  memcpy(darr->data + darr->used, bytes, n);
  darr->used += n;
}

byte_t *darr_at(darr_t *darr, size_t index)
{
  if (index >= darr->used)
    return NULL;
  return darr->data + index;
}

void darr_write_file(darr_t *bytes, FILE *fp)
{
  size_t size = fwrite(bytes->data, bytes->used, 1, fp);
  assert(size == 1);
}

darr_t darr_read_file(FILE *fp)
{
  darr_t darr = {0};
  (void)fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  darr_init(&darr, size);
  (void)fseek(fp, 0, SEEK_SET);
  (void)fread(darr.data, size, 1, fp);
  return darr;
}
