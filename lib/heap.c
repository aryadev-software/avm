/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-11-01
 * Author: Aryadev Chavali
 * Description: Arena allocator
 */

#include "./heap.h"

page_t *page_create(size_t max, page_t *next)
{
  page_t *page    = calloc(1, sizeof(*page) + max);
  page->used      = 0;
  page->available = max;
  page->next      = next;
  return page;
}

size_t page_space_left(page_t *page)
{
  return WORD_SAFE_SUB(page->available, page->used);
}

void page_delete(page_t *page)
{
  free(page);
}

void heap_create(heap_t *heap)
{
  heap->beg   = page_create(PAGE_DEFAULT_SIZE, NULL);
  heap->end   = heap->beg;
  heap->pages = 1;
}

byte *heap_allocate(heap_t *heap, size_t requested)
{
  page_t *ptr = heap->beg;
  while (page_space_left(ptr) < requested)
    ptr = ptr->next;
  if (ptr)
  {
    byte *data = ptr->used + ptr->data;
    ptr->used += requested;
    return data;
  }
  // Otherwise we are at the end of the heap, and we need to allocate
  // a new page

  // Create new page and get the pointer to the start of requested data
  page_t *new_page = page_create(MAX(PAGE_DEFAULT_SIZE, requested), NULL);
  byte *data       = new_page->used + new_page->data;

  // Set the end of the heap to the new page (update linked list)
  heap->end->next = new_page;
  heap->end       = new_page;
  new_page->used += requested;
  ++heap->pages;

  return data;
}

void heap_stop(heap_t *heap)
{
  page_t *ptr = heap->beg;
  for (size_t i = 0; i < heap->pages; ++i)
  {
    page_t *cur  = ptr;
    page_t *next = ptr->next;
    page_delete(cur);
    ptr = next;
  }
  heap->beg   = NULL;
  heap->end   = NULL;
  heap->pages = 0;
}
