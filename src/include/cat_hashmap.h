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

#ifndef __CAT_HASHMAP_H__
#define __CAT_HASHMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "cat_error.h"

typedef struct hashmap_s* hashmap_t;

size_t hashmap_size(hashmap_t ht);
size_t hashmap_capacity(hashmap_t ht);
double hashmap_load(hashmap_t ht);

int hashmap_is_empty(hashmap_t ht);
int hashmap_contains_key(hashmap_t ht, void* key);
size_t hashmap_contains_val(hashmap_t ht, void* val);

hashmap_t _hashmap_init(size_t capacity,
                        size_t key_len,
                        size_t elem_size,
                        uint64_t (*hash_fn)(const char*),
                        int (*cmp_fn)(const void*, const void*),
                        void* (*alloc_fn)(size_t),
                        void (*free_fn)(void*));
stat_t hashmap_reserve(hashmap_t ht, size_t capacity);
stat_t hashmap_assign(hashmap_t ht, void* key, void* val);
stat_t hashmap_remove(hashmap_t ht, void* key, void* ret_val);
stat_t hashmap_query(hashmap_t ht, void* key, void* ret_val);

stat_t hashmap_copy(hashmap_t* dst, hashmap_t src);

void hashmap_map(hashmap_t ht, void (*fn)(void*, void*));
void hashmap_key_map(hashmap_t ht, void (*fn)(void*));
void hashmap_val_map(hashmap_t ht, void (*fn)(void*));
void hashmap_clear(hashmap_t ht);
void hashmap_deinit(hashmap_t ht);

uint64_t cityhash64(const char *s, size_t len);
uint64_t djb2hash64(const char *s, size_t len);

#define hashmap(key_type, val_type, capacity) \
    _hashmap_init(capacity, \
                  sizeof(key_type), \
                  sizeof(val_type), \
                  NULL, \
                  NULL, \
                  NULL, \
                  NULL)

#define hashmap_custom(key_type, val_type, capacity, ...) \
    _hashmap_init(capacity, \
                  sizeof(key_type), \
                  sizeof(val_type), \
                  ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif