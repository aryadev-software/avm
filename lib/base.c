/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2023-10-26
 * Author: Aryadev Chavali
 * Description: Implementation of basic library functions
 */

#include "./base.h"

#include <string.h>

short_t short_byteswap(const short_t w)
{
  // TODO: is there a faster way of doing this?
  return WORD_NTH_BYTE(w, 1) | (WORD_NTH_BYTE(w, 0) << 8);
}

hword_t hword_byteswap(const hword_t w)
{
  // TODO: is there a faster way of doing this?
  return WORD_NTH_BYTE(w, 3) | (WORD_NTH_BYTE(w, 2) << 8) |
         WORD_NTH_BYTE(w, 1) << 16 | WORD_NTH_BYTE(w, 0) << 24;
}

word_t word_byteswap(const word_t w)
{
  // TODO: is there a faster way of doing this?
  return WORD_NTH_BYTE(w, 7) | WORD_NTH_BYTE(w, 6) << 8 |
         WORD_NTH_BYTE(w, 5) << 16 | WORD_NTH_BYTE(w, 4) << 24 |
         WORD_NTH_BYTE(w, 3) << 32 | WORD_NTH_BYTE(w, 2) << 40 |
         WORD_NTH_BYTE(w, 1) << 48 | WORD_NTH_BYTE(w, 0) << 56;
}

short_t convert_bytes_to_short(const byte_t *bytes)
{
  short_t s = 0;
  memcpy(&s, bytes, SHORT_SIZE);
  if (!LITTLE_ENDIAN)
    s = short_byteswap(s);
  return s;
}

hword_t convert_bytes_to_hword(const byte_t *bytes)
{
  hword_t h = 0;
  memcpy(&h, bytes, HWORD_SIZE);
  if (!LITTLE_ENDIAN)
    h = hword_byteswap(h);
  return h;
}

word_t convert_bytes_to_word(const byte_t *bytes)
{
  word_t h = 0;
  memcpy(&h, bytes, WORD_SIZE);
  if (!LITTLE_ENDIAN)
    h = word_byteswap(h);
  return h;
}

void convert_short_to_bytes(short_t w, byte_t *bytes)
{
  short_t h = LITTLE_ENDIAN ? w : short_byteswap(w);
  memcpy(bytes, &h, SHORT_SIZE);
}

void convert_hword_to_bytes(hword_t w, byte_t *bytes)
{
  hword_t h = LITTLE_ENDIAN ? w : hword_byteswap(w);
  memcpy(bytes, &h, HWORD_SIZE);
}

void convert_word_to_bytes(word_t w, byte_t *bytes)
{
  word_t h = LITTLE_ENDIAN ? w : word_byteswap(w);
  memcpy(bytes, &h, WORD_SIZE);
}
