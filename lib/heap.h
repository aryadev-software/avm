/* Copyright (C) 2023, 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.

 * Created: 2023-11-01
 * Author: Aryadev Chavali
 * Description: Arena allocator
 */

#ifndef HEAP_H
#define HEAP_H

#include "./base.h"
#include "./darr.h"

#include <stdbool.h>

#define PAGE_DEFAULT_SIZE 256

/**
   @brief Some fixed portion of bytes allocated on the heap.

   @details A fixed allocation of bytes.  Cannot be resized nor can it
   be stack allocated (the usual way) due to flexible array attached.

   @prop[next] Next page in the linked list
   @prop[available] Available number of bytes in page
   @prop[data] Buffer of bytes attached to page
 */
typedef struct Page
{
  size_t available;
  byte_t data[];
} page_t;

/**
   @brief Allocate a new page on the heap with the given properties.

   @details Allocates a new page using malloc with the given size and
   pointer to next page.  NOTE: all memory is 0 initialised by
   default.

   @param[max] Maximum available memory in page
 */
page_t *page_create(size_t max);

/**
   @brief Delete a page, freeing its memory

   @details Free's the memory associated with the page via free().
   NOTE: any pointers to the page's memory are considered invalid once
   this is called.

   @param[page] Page to delete
 */
void page_delete(page_t *page);

/**
   @brief A collection of pages through which generic allocations can
   occur.

   @details Collection of pages maintained through a vector of
   pointers to pages.

   @prop[page_vec] Vector of pages
 */
typedef struct
{
  darr_t page_vec;
} heap_t;

#define HEAP_SIZE(HEAP) ((HEAP).page_vec.used / sizeof(page_t *))

/**
   @brief Instantiate a new heap structure

   @details Initialises the heap structure given.  No heap allocation
   occurs here until a new page is created, so this may be called
   safely.

   @param[heap] Pointer to heap to initialise
 */
void heap_create(heap_t *heap);

/**
   @brief Allocate a new page on the heap

   @details Creates and joins a new page onto the linked list
   maintained by the heap.  heap.end is set to this new page.

   @param[heap] Heap to create a new page on
   @param[size] Size of page to allocate

   @return The newly allocated page
 */
page_t *heap_allocate(heap_t *heap, size_t size);

/**
   @brief Free a page of memory from the heap

   @details The page given is removed from the linked list of pages
   then freed from the heap via page_delete().  If the page does not
   belong to this heap (O(heap.pages) time) then false is returned,
   otherwise true.

   @param[heap] Heap to free page from
   @param[page] Page to delete

   @return Success of deletion
 */
bool heap_free(heap_t *heap, page_t *page);

/**
   @brief Stop the heap, freeing all associated memory

   @details Traverses the linked list of pages, deleting each one.

   @param[heap] Heap to stop
 */
void heap_stop(heap_t *);

#endif
