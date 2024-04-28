/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-11-01
 * Author: Aryadev Chavali
 * Description: Arena allocator
 */

#ifndef HEAP_H
#define HEAP_H

#include "./base.h"

#include <stdbool.h>
#include <stdlib.h>

#define PAGE_DEFAULT_SIZE 256

/**
   @brief Some fixed portion of bytes allocated on the heap in a
   linked list.

   @details A fixed allocation of bytes, with size and a link to the
   next page.  Cannot be resized nor can it be stack allocated the
   usual way due to flexible array attached.

   @prop[next] Next page in the linked list
   @prop[available] Available number of bytes in page
   @prop[data] Buffer of bytes attached to page
 */
typedef struct Page
{
  struct Page *next;
  size_t available;
  byte_t data[];
} page_t;

/**
   @brief Allocate a new page on the heap with the given properties.

   @details Allocates a new page using malloc with the given size and
   pointer to next page.  NOTE: all memory is 0 initialised by
   default.

   @param[max] Maximum available memory in page
   @param[next] Next page to link this page to
 */
page_t *page_create(size_t max, page_t *next);

/**
   @brief Delete a page, freeing its memory

   @details Free's the memory associated with the page via free().
   NOTE: any pointer's to the page's memory are considered invalid
   once this is called.

   @param[page] Page to delete
 */
void page_delete(page_t *page);

/**
   @brief A collection of pages through which generic allocations can
   occur.

   @details Structure which maintains a linked list of pages (with a
   reference to the beginning and end of it).

   @prop[beg] Beginning of linked list of pages
   @prop[end] End of linked list of pages
   @prop[pages] Number of pages allocated in heap
 */
typedef struct
{
  page_t *beg, *end;
  size_t pages;
} heap_t;

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
