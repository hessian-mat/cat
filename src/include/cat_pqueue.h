/* The MIT License

   Copyright (c) 2024 hessian-mat

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#ifndef __CAT_PQUEUE_H__
#define __CAT_PQUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "cat_error.h"

typedef struct pqueue_s* pqueue_t;

size_t pqueue_size(pqueue_t pq);
size_t pqueue_capacity(pqueue_t pq);

int pqueue_is_empty(pqueue_t pq);
int pqueue_is_full(pqueue_t pq);
size_t pqueue_contains(pqueue_t pq, void* elem);

pqueue_t _pqueue_init(size_t capacity,
                      size_t elem_size,
                      int (*cmp_fn)(const void*, const void*),
                      void (*free_fn)(void*),
                      void* (*alloc_fn)(size_t));
stat_t pqueue_reserve(pqueue_t pq, size_t capacity);
stat_t pqueue_shrink_to_fit(pqueue_t pq);
stat_t pqueue_push(pqueue_t pq, void* elem);
stat_t pqueue_pop(pqueue_t pq, void* ret_elem);
stat_t pqueue_top(pqueue_t pq, void* ret_elem);

stat_t pqueue_merge(pqueue_t dst, pqueue_t src);
stat_t pqueue_copy(pqueue_t* dst, pqueue_t src);

void pqueue_map(pqueue_t pq, void (*fn)(void*));
void pqueue_clear(pqueue_t pq);
void pqueue_deinit(pqueue_t pq);

#define pqueue(type, capacity, cmp) \
    _pqueue_init(capacity, \
                 sizeof(type), \
                 cmp, \
                 NULL, \
                 NULL)

#define pqueue_custom(type, capacity, cmp, ...) \
    _pqueue_init(capacity, \
                 sizeof(type), \
                 cmp, \
                 ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
