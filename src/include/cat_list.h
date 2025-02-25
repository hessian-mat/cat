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

#ifndef __CAT_LIST_H__
#define __CAT_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "cat_error.h"

typedef struct list_s* list_t;
typedef struct list_it_s* list_it_t;

size_t list_size(list_t list);

int list_is_empty(list_t list);
size_t list_contains(list_t list,
                     void* elem,
                     int (*cmp_fn)(const void*, const void*));

list_t _list_init(size_t elem_size,
                  void (*free_fn)(void*),
                  void* (*alloc_fn)(size_t));
stat_t list_push_front(list_t list, void* elem);
stat_t list_push_back(list_t list, void* elem);
stat_t list_pop_front(list_t list, void* ret_elem);
stat_t list_pop_back(list_t list, void* ret_elem);
stat_t list_insert(list_t list, void* elem, size_t i);
stat_t list_remove(list_t list, void* ret_elem, size_t i);
stat_t list_front(list_t list, void* ret_elem);
stat_t list_back(list_t list, void* ret_elem);
stat_t list_get(list_t list, void* ret_elem, size_t i);
stat_t list_set(list_t list, void* elem, size_t i);

void list_concat(list_t dst, list_t src);
stat_t list_copy(list_t* dst, list_t src);

void list_map(list_t list, void (*fn)(void*));
void list_clear(list_t list);
void list_deinit(list_t list);

list_it_t list_begin(list_t list);
list_it_t list_end(list_t list);
int list_next(list_it_t it, void* ret_elem);
int list_prev(list_it_t it, void* ret_elem);

void list_it_deinit(list_it_t it);

#define list(type) \
    _list_init(sizeof(type), \
               NULL, \
               NULL)

#define list_custom(type, ...) \
    _list_init(sizeof(type), \
               ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
