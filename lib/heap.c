/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2023-11-01
 * Author: Aryadev Chavali
 * Description: Arena allocator
 */

#include "./heap.h"

#include <lib/darr.h>

#include <malloc.h>
#include <stdio.h>
#include <string.h>

page_t *page_create(size_t max)
{
  if (max == 0)
    max = PAGE_DEFAULT_SIZE;

  page_t *page    = calloc(1, sizeof(*page) + max);
  page->available = max;
  return page;
}

void page_delete(page_t *page)
{
  free(page);
}

void heap_create(heap_t *heap)
{
  memset(heap, 0, sizeof(*heap));
}

page_t *heap_allocate(heap_t *heap, size_t requested)
{
  page_t *cur = page_create(requested);
  darr_append_bytes(&heap->page_vec, (byte_t *)cur, sizeof(cur));
  return cur;
}

bool heap_free(heap_t *heap, page_t *page)
{
  if (!page || !heap)
    return false;

  for (size_t i = 0; i < (heap->page_vec.used / sizeof(page)); ++i)
  {
    page_t *cur = DARR_AT(page_t *, heap->page_vec.data, i);
    if (cur == page)
    {
      page_delete(cur);
      // TODO: When does this fragmentation become a performance issue?
      DARR_AT(page_t *, heap->page_vec.data, i) = NULL;
      return true;
    }
  }

  return false;
}

void heap_stop(heap_t *heap)
{
  for (size_t i = 0; i < (heap->page_vec.used / sizeof(page_t *)); i++)
  {
    page_t *ptr = DARR_AT(page_t *, heap->page_vec.data, i);
    if (ptr)
      page_delete(ptr);
  }
  free(heap->page_vec.data);
  heap->page_vec = (darr_t){0};
}
