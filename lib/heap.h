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

#include <stdlib.h>

#define PAGE_DEFAULT_SIZE 64

typedef struct Page
{
  struct Page *next;
  size_t used, available;
  byte data[];
} page_t;

page_t *page_create(size_t, page_t *);
size_t page_space_left(page_t *);
void page_delete(page_t *);

typedef struct
{
  page_t *beg, *end;
  size_t pages;
} heap_t;

void heap_create(heap_t *);
byte *heap_allocate(heap_t *, size_t);
void heap_stop(heap_t *);

#endif
