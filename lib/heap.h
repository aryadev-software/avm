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

typedef struct Page
{
  struct Page *next;
  size_t available;
  byte data[];
} page_t;

page_t *page_create(size_t, page_t *);
void page_delete(page_t *);

typedef struct
{
  page_t *beg, *end;
  size_t pages;
} heap_t;

void heap_create(heap_t *);
bool heap_free_page(heap_t *, page_t *);
page_t *heap_allocate(heap_t *, size_t);
void heap_stop(heap_t *);

#endif
