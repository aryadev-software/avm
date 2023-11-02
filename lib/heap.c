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

#include <stdio.h>

page_t *page_create(size_t max, page_t *next)
{
  page_t *page    = calloc(1, sizeof(*page) + max);
  page->available = max;
  page->next      = next;
  return page;
}

void page_delete(page_t *page)
{
  free(page);
}

void heap_create(heap_t *heap)
{
  heap->beg = heap->end = NULL;
  heap->pages           = 0;
}

bool heap_free_page(heap_t *heap, page_t *page)
{
  if (!page || !heap)
    return false;

  if (page == heap->beg)
  {
    heap->beg = heap->beg->next;
    page_delete(page);
    --heap->pages;
    if (heap->pages == 0)
      heap->end = NULL;
    return true;
  }

  page_t *prev = NULL, *next = NULL, *cur = NULL;
  for (cur = heap->beg; cur; cur = cur->next)
  {
    next = cur->next;
    if (cur == page)
      break;
    prev = cur;
  }

  if (!cur)
    // Couldn't find the page
    return false;
  // Page was found
  prev->next = next;
  if (!next)
    // This means page == heap->end
    heap->end = prev;
  page_delete(page);
  --heap->pages;
  if (heap->pages == 0)
    heap->beg = NULL;

  return true;
}

page_t *heap_allocate(heap_t *heap, size_t requested)
{
  page_t *cur = page_create(requested, NULL);
  if (heap->end)
    heap->end->next = cur;
  else
    heap->beg = cur;
  heap->end = cur;
  heap->pages++;
  return cur;
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
