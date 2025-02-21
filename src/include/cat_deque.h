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

#ifndef __CAT_DEQUE_H__
#define __CAT_DEQUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "cat_error.h"

typedef struct deque_s* deque_t;

size_t deque_size(deque_t deq);
size_t deque_capacity(deque_t deq);

int deque_is_empty(deque_t deq);
int deque_is_full(deque_t deq);
size_t deque_contains(deque_t deq,
                      void* elem,
                      int (*cmp_fn)(const void*, const void*));

deque_t _deque_init(size_t capacity,
                    size_t elem_size,
                    void (*free_fn)(void*),
                    void* (*alloc_fn)(size_t));
stat_t deque_reserve(deque_t deq, size_t capacity);
stat_t deque_shrink_to_fit(deque_t deq);
stat_t deque_push_front(deque_t deq, void* elem);
stat_t deque_push_back(deque_t deq, void* elem);
stat_t deque_pop_front(deque_t deq, void* ret_elem);
stat_t deque_pop_back(deque_t deq, void* ret_elem);
stat_t deque_front(deque_t deq, void* ret_elem);
stat_t deque_back(deque_t deq, void* ret_elem);
stat_t deque_get(deque_t deq, void* ret_elem, size_t i);
stat_t deque_set(deque_t deq, void* elem, size_t i);

stat_t deque_copy(deque_t* dst, deque_t src);

void* deque_at(deque_t deq, size_t i);
void deque_map(deque_t deq, void (*fn)(void*));
void deque_clear(deque_t deq);
void deque_deinit(deque_t deq);

#define deque(type, capacity) \
    _deque_init(capacity, \
                sizeof(type), \
                NULL, \
                NULL)

#define deque_custom(type, capacity, ...) \
    _deque_init(capacity, \
                sizeof(type), \
                ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
