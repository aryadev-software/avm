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

void byteswap(byte_t *bytes, size_t size)
{
  for (size_t i = 0; i < size; ++i)
  {
    byte_t b            = bytes[i];
    bytes[i]            = bytes[size - i - 1];
    bytes[size - i - 1] = b;
  }
}

void convert_bytes_le(byte_t *bytes, size_t size_bytes)
{
  if (!LITTLE_ENDIAN)
    byteswap(bytes, size_bytes);
}

void print_byte_array(FILE *fp, const byte_t *bytes, size_t size_bytes)
{
  for (size_t i = 0; i < size_bytes; ++i)
  {
    fprintf(fp, "0x%s%X", (bytes[i] < 16 ? "_" : ""), bytes[i]);
    if (i != size_bytes - 1)
      fputs(", ", fp);
  }
}
