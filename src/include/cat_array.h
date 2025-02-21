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

#ifndef __CAT_ARRAY_H__
#define __CAT_ARRAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "cat_error.h"

typedef struct array_s* array_t;

size_t array_size(array_t arr);
size_t array_capacity(array_t arr);
void* array_data(array_t arr);

int array_is_empty(array_t arr);
int array_is_full(array_t arr);
size_t array_contains(array_t arr,
                      void* elem,
                      int (*cmp_fn)(const void*, const void*));

array_t _array_init(size_t capacity,
                    size_t elem_size,
                    void (*free_fn)(void*),
                    void* (*alloc_fn)(size_t));
stat_t array_reserve(array_t arr, size_t capacity);
stat_t array_shrink_to_fit(array_t arr);
stat_t array_push_back(array_t arr, void* elem);
stat_t array_pop_back(array_t arr, void* ret_elem);
stat_t array_insert(array_t arr, void* elem, size_t i);
stat_t array_remove(array_t arr, void* ret_elem, size_t i);
stat_t array_front(array_t arr, void* ret_elem);
stat_t array_back(array_t arr, void* ret_elem);
stat_t array_get(array_t arr, void* ret_elem, size_t i);
stat_t array_set(array_t arr, void* elem, size_t i);

stat_t array_copy(array_t* dst, array_t src);

void* array_at(array_t arr, size_t i);
void* array_bsearch(array_t arr,
                    void* elem,
                    int (*cmp_fn)(const void*, const void*));
void array_qsort(array_t arr,
                 int (*cmp_fn)(const void*, const void*));
void array_map(array_t arr, void (*fn)(void*));
void array_clear(array_t arr);
void array_deinit(array_t arr);

#define array(type, capacity) \
    _array_init(capacity, \
                sizeof(type), \
                NULL, \
                NULL)

#define array_custom(type, capacity, ...) \
    _array_init(capacity, \
                sizeof(type), \
                ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
