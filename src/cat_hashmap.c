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

#include "cat_hashmap.h"

#include <stdlib.h>
#include <string.h>

#define HASHMAP_LOAD_THRESHOLD 0.75
#define HASHMAP_MIN_CAPACITY 8
#define HASHMAP_MAX_CAPACITY (SIZE_MAX / 2 + 1)

typedef struct hashmap_entry_s {
    struct hashmap_entry_s     *next;
    uint64_t                    hash;
    void                       *key;
    void                       *elem;
} hashmap_entry_s, *hashmap_entry_t;

typedef struct hashmap_s {
    struct hashmap_entry_s    **entries;
    size_t                      size;
    size_t                      capacity;
    size_t                      elem_size;
    size_t                      key_len;

    uint64_t                  (*hash_fn)(const char*);
    int                       (*cmp_fn)(const void*, const void*);
    void                      (*free_fn)(void*);
    void                     *(*alloc_fn)(size_t);
} hashmap_s;

static stat_t null_entry_assign(hashmap_t ht, void* val);
static stat_t null_entry_remove(hashmap_t ht, void* ret_val);
static stat_t null_entry_query(hashmap_t ht, void* ret_val);

static stat_t hashmap_alloc(hashmap_t ht, size_t capacity);
static void hashmap_rehash(hashmap_t ht,
                           hashmap_entry_t* entries,
                           size_t capacity);

static stat_t entry_alloc(hashmap_t ht, hashmap_entry_t* entry);
static stat_t null_entry_alloc(hashmap_t ht, hashmap_entry_t* entry);

static size_t roundup_pow2(size_t n);

static uint64_t hashmap_hash(hashmap_t ht, void* key);

/**
 * Get the size of the hashmap
 * 
 * @param ht Hashmap
 * @return Size of the hashmap
 */
size_t hashmap_size(hashmap_t ht)
{
    return ht->size;
}

/**
 * Get the capacity of the hashmap
 * 
 * @param ht Hashmap
 * @return Capacity of the hashmap
 */
size_t hashmap_capacity(hashmap_t ht)
{
    return ht->capacity;
}

/**
 * Get the load factor of the hashmap
 * 
 * @param ht Hashmap
 * @return Load factor of the hashmap
 */
double hashmap_load(hashmap_t ht)
{
    return (double)ht->size / ht->capacity;
}

/**
 * Check if the hashmap is empty
 * 
 * @param ht Hashmap
 * @return 1 if the hashmap is empty, 0 otherwise
 */
int hashmap_is_empty(hashmap_t ht)
{
    return ht->size == 0;
}

/**
 * Check if a key is in the hashmap
 * 
 * @param ht Hashmap
 * @param key Key to check
 * @return 1 if the key is in the hashmap, 0 otherwise
 */
int hashmap_contains_key(hashmap_t ht, void* key)
{
    if (!key) {
        hashmap_entry_t entry = ht->entries[0];
        while (entry) {
            if (!entry->key) return 1;
            entry = entry->next;
        }
    } else {
        uint64_t hash = hashmap_hash(ht, key);
        size_t i = hash & (ht->capacity - 1);
        hashmap_entry_t entry = ht->entries[i];
        while (entry) {
            if (entry->key) {
                int match = ht->cmp_fn ?
                    ht->cmp_fn(entry->key, key) :
                    memcmp(entry->key, key, ht->key_len);
                if (match == 0) return 1;
            }
            entry = entry->next;
        }
    }
    return 0;
}

/**
 * Check if a value is in the hashmap
 * 
 * @param ht Hashmap
 * @param val Value to check
 * @return Number of times the value is in the hashmap
 */
size_t hashmap_contains_val(hashmap_t ht, void* val)
{
    size_t count = 0;
    for (size_t i = 0; i < ht->capacity; i++) {
        hashmap_entry_t entry = ht->entries[i];
        while (entry) {
            int match = ht->cmp_fn ?
                ht->cmp_fn(entry->elem, val) :
                memcmp(entry->elem, val, ht->elem_size);
            if (match == 0) count++;
            entry = entry->next;
        }
    }
    return count;
}

/**
 * Allocate or reallocate memory for the hashmap entries
 * 
 * @param ht Hashmap
 * @param capacity Capacity of the hashmap
 * @return COMPLETE on success, corresponding error code on failure
 */
static stat_t hashmap_alloc(hashmap_t ht, size_t capacity)
{
    hashmap_entry_t* entries = NULL;
    void* (*alloc)(size_t) = ht->alloc_fn ? ht->alloc_fn : malloc;
    void (*dealloc)(void*) = ht->free_fn ? ht->free_fn : free;

    entries = alloc(capacity * sizeof(hashmap_entry_t));
    if (!entries) return ERR_MEMORY_ALLOCATION;
    memset(entries, 0, capacity * sizeof(hashmap_entry_t));

    if (ht->entries) {
        hashmap_rehash(ht, entries, capacity);
        dealloc(ht->entries);
    }
    ht->entries = entries;
    return COMPLETE;
}

/**
 * Rehash the hashmap entries
 * 
 * @param ht Hashmap
 * @param entries New entries to rehash into
 * @param capacity New capacity of the hashmap
 */
static void hashmap_rehash(hashmap_t ht, hashmap_entry_t* entries, size_t capacity)
{
    for (size_t i = 0; i < ht->capacity; i++) {
        hashmap_entry_t entry = ht->entries[i];
        while (entry) {
            hashmap_entry_t next = entry->next;
            size_t j = entry->hash & (capacity - 1);
            entry->next = entries[j];
            entries[j] = entry;
            entry = next;
        }
    }
}

/**
 * Initialize a hashmap
 * 
 * @param capacity Initial capacity of the hashmap
 * @param key_len Length of the key
 * @param elem_size Size of each element in the hashmap
 * @param hash_fn Hash function, NULL for default cityhash64
 * @param cmp_fn Comparison function, NULL for default memcmp
 * @param alloc_fn Allocation function, NULL for default malloc
 * @param free_fn Free function, NULL for default free
 * @return Initialized hashmap on success, NULL on failure
 */
hashmap_t _hashmap_init(size_t capacity,
                        size_t key_len,
                        size_t elem_size,
                        uint64_t (*hash_fn)(const char*),
                        int (*cmp_fn)(const void*, const void*),
                        void* (*alloc_fn)(size_t),
                        void (*free_fn)(void*))
{
    if (capacity > ((size_t)0 - 1) / elem_size)
        return NULL;
    hashmap_t ht = (hashmap_t)malloc(sizeof(hashmap_s));
    if (!ht) return NULL;

    ht->capacity = capacity <= HASHMAP_MIN_CAPACITY ?
                   HASHMAP_MIN_CAPACITY : roundup_pow2(capacity);
    ht->size = 0;
    ht->elem_size = elem_size;
    ht->key_len = key_len;
    ht->entries = NULL;

    ht->hash_fn = hash_fn;
    ht->cmp_fn = cmp_fn;
    ht->alloc_fn = alloc_fn;
    ht->free_fn = free_fn;

    if (hashmap_alloc(ht, ht->capacity)) {
        free(ht);
        return NULL;
    }
    return ht;
}

/**
 * Reserve memory for the hashmap
 * 
 * @param ht Hashmap
 * @param capacity Capacity of the buffer to reserve
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t hashmap_reserve(hashmap_t ht, size_t capacity)
{
    if (capacity <= ht->capacity ||
        capacity >= HASHMAP_MAX_CAPACITY >> 1)
        return ERR_INVALID_OPERATION;
    size_t new = roundup_pow2(capacity);
    if (hashmap_alloc(ht, new))
        return ERR_MEMORY_ALLOCATION;
    ht->capacity = new;
    return COMPLETE;
}

/**
 * Allocate memory for an entry
 * 
 * @param ht Hashmap
 * @param entry Pointer to the entry to allocate
 * @return COMPLETE on success, corresponding error code on failure
 */
static stat_t entry_alloc(hashmap_t ht, hashmap_entry_t* entry)
{
    void* (*alloc)(size_t) = ht->alloc_fn ? ht->alloc_fn : malloc;
    void (*dealloc)(void*) = ht->free_fn ? ht->free_fn : free;

    *entry = (hashmap_entry_t)alloc(sizeof(hashmap_entry_s));
    if (!*entry) return ERR_MEMORY_ALLOCATION;

    (*entry)->key = alloc(ht->key_len);
    if (!(*entry)->key) {
        dealloc(*entry);
        return ERR_MEMORY_ALLOCATION;
    }

    (*entry)->elem = alloc(ht->elem_size);
    if (!(*entry)->elem) {
        dealloc((*entry)->key);
        dealloc(*entry);
        return ERR_MEMORY_ALLOCATION;
    }

    (*entry)->next = NULL;
    return COMPLETE;
}

/**
 * Allocate memory for a null entry
 * 
 * @param ht Hashmap
 * @param entry Pointer to the entry to allocate
 * @return COMPLETE on success, corresponding error code on failure
 */
static stat_t null_entry_alloc(hashmap_t ht, hashmap_entry_t* entry)
{
    void* (*alloc)(size_t) = ht->alloc_fn ? ht->alloc_fn : malloc;
    void (*dealloc)(void*) = ht->free_fn ? ht->free_fn : free;

    *entry = (hashmap_entry_t)alloc(sizeof(hashmap_entry_s));
    if (!*entry) return ERR_MEMORY_ALLOCATION;
    
    (*entry)->elem = alloc(ht->elem_size);
    if (!(*entry)->elem) {
        dealloc(*entry);
        return ERR_MEMORY_ALLOCATION;
    }

    (*entry)->key = NULL;
    (*entry)->hash = 0;
    (*entry)->next = NULL;
    return COMPLETE;
}

/**
 * Insert or update a mapping for NULL into the hashmap
 * 
 * @param ht Hashmap
 * @param val Value to insert
 * @return COMPLETE on success, corresponding error code on failure
 */
static stat_t null_entry_assign(hashmap_t ht, void* val)
{
    hashmap_entry_t entry = ht->entries[0];
    while (entry) {
        if (!entry->key) {
            memcpy(entry->elem, val, ht->elem_size);
            return COMPLETE;
        }
        entry = entry->next;
    }

    hashmap_entry_t null_entry = NULL;
    if (null_entry_alloc(ht, &null_entry))
        return ERR_MEMORY_ALLOCATION;
    null_entry->next = ht->entries[0];
    memcpy(null_entry->elem, val, ht->elem_size);
    ht->entries[0] = null_entry;
    ht->size++;

    return COMPLETE;
}

/**
 * Remove a mapping for NULL from the hashmap
 * 
 * @param ht Hashmap
 * @param ret_val Pointer to the value to return
 * @return COMPLETE on success, corresponding error code on failure
 */
static stat_t null_entry_remove(hashmap_t ht, void* ret_val)
{
    hashmap_entry_t entry = ht->entries[0];
    hashmap_entry_t prev = NULL;

    while (entry) {
        if (!entry->key) {
            if (prev) prev->next = entry->next;
            else ht->entries[0] = entry->next;
            if (ret_val)
                memcpy(ret_val, entry->elem, ht->elem_size);
            if (ht->free_fn) {
                ht->free_fn(entry->elem);
                ht->free_fn(entry);
            } else {
                free(entry->elem);
                free(entry);
            }
            ht->size--;
            return COMPLETE;
        }
        prev = entry;
        entry = entry->next;
    }
    return ERR_INVALID_OPERATION;
}

/**
 * Query a mapping for NULL from the hashmap
 * 
 * @param ht Hashmap
 * @param ret_val Pointer to the value to return
 * @return COMPLETE on success, corresponding error code on failure
 */
static stat_t null_entry_query(hashmap_t ht, void* ret_val)
{
    hashmap_entry_t entry = ht->entries[0];
    while (entry) {
        if (!entry->key) {
            memcpy(ret_val, entry->elem, ht->elem_size);
            return COMPLETE;
        }
        entry = entry->next;
    }
    return ERR_INVALID_OPERATION;
}

/**
 * Get the hash of a key
 * 
 * @param ht Hashmap
 * @param key Key to hash
 * @return Hash of the key
 */
static uint64_t hashmap_hash(hashmap_t ht, void* key)
{
    if (ht->hash_fn) return ht->hash_fn((char*)key);
    return cityhash64((char*)key, ht->key_len);
}

/**
 * Insert or update a mapping into the hashmap
 * 
 * @param ht Hashmap
 * @param key Key to insert
 * @param val Value to insert
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t hashmap_assign(hashmap_t ht, void* key, void* val)
{
    if (hashmap_load(ht) >= HASHMAP_LOAD_THRESHOLD) {
        if (ht->capacity << 1 >= HASHMAP_MAX_CAPACITY)
            return ERR_CAPACITY_OVERFLOW;
        if (hashmap_alloc(ht, ht->capacity << 1))
            return ERR_MEMORY_ALLOCATION;
        ht->capacity <<= 1;
    }
    if (!key) return null_entry_assign(ht, val);

    uint64_t hash = hashmap_hash(ht, key);
    size_t i = hash & (ht->capacity - 1);

    hashmap_entry_t entry = ht->entries[i];
    while (entry) {
        if (entry->key) {
            int match = ht->cmp_fn ?
                ht->cmp_fn(entry->key, key) :
                memcmp(entry->key, key, ht->key_len);
            if (match == 0) {
                memcpy(entry->elem, val, ht->elem_size);
                return COMPLETE;
            }
        }
        entry = entry->next;
    }

    hashmap_entry_t new_entry = NULL;
    if (entry_alloc(ht, &new_entry))
        return ERR_MEMORY_ALLOCATION;
    memcpy(new_entry->key, key, ht->key_len);
    new_entry->hash = hash;
    new_entry->next = ht->entries[i];
    memcpy(new_entry->elem, val, ht->elem_size);

    ht->entries[i] = new_entry;
    ht->size++;
    return COMPLETE;
}

/**
 * Remove a mapping from the hashmap
 * 
 * @param ht Hashmap
 * @param key Key to remove
 * @param ret_val Pointer to the value to return
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t hashmap_remove(hashmap_t ht, void* key, void* ret_val)
{
    if (!key) return null_entry_remove(ht, ret_val);

    uint64_t hash = hashmap_hash(ht, key);
    size_t i = hash & (ht->capacity - 1);

    hashmap_entry_t entry = ht->entries[i];
    hashmap_entry_t prev = NULL;
    while (entry) {
        if (entry->key) {
            int match = ht->cmp_fn ?
                ht->cmp_fn(entry->key, key) :
                memcmp(entry->key, key, ht->key_len);
            if (match == 0) {
                if (prev) prev->next = entry->next;
                else ht->entries[i] = entry->next;
                if (ret_val)
                    memcpy(ret_val, entry->elem, ht->elem_size);
                if (ht->free_fn) {
                    ht->free_fn(entry->key);
                    ht->free_fn(entry->elem);
                    ht->free_fn(entry);
                } else {
                    free(entry->key);
                    free(entry->elem);
                    free(entry);
                }
                ht->size--;
                return COMPLETE;
            }
        }
        prev = entry;
        entry = entry->next;
    }
    return ERR_INVALID_OPERATION;
}

/**
 * Query a mapping from the hashmap
 * 
 * @param ht Hashmap
 * @param key Key to query
 * @param ret_val Pointer to the value to return
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t hashmap_query(hashmap_t ht, void* key, void* ret_val)
{
    if (!key) return null_entry_query(ht, ret_val);

    uint64_t hash = hashmap_hash(ht, key);
    size_t i = hash & (ht->capacity - 1);

    hashmap_entry_t entry = ht->entries[i];
    while (entry) {
        if (entry->key) {
            int match = ht->cmp_fn ?
                ht->cmp_fn(entry->key, key) :
                memcmp(entry->key, key, ht->key_len);
            if (match == 0) {
                memcpy(ret_val, entry->elem, ht->elem_size);
                return COMPLETE;
            }
        }
        entry = entry->next;
    }
    return ERR_INVALID_OPERATION;
}

/**
 * Copy a hashmap
 * 
 * @param dst Pointer to the destination hashmap
 * @param src Source hashmap
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t hashmap_copy(hashmap_t* dst, hashmap_t src)
{
    *dst = _hashmap_init(src->capacity,
                         src->key_len,
                         src->elem_size,
                         src->hash_fn,
                         src->cmp_fn,
                         src->alloc_fn,
                         src->free_fn);
    if (!*dst) return ERR_MEMORY_ALLOCATION;

    for (size_t i = 0; i < src->capacity; i++) {
        hashmap_entry_t entry = src->entries[i];
        while (entry) {
            if (hashmap_assign(*dst, entry->key, entry->elem)) {
                hashmap_deinit(*dst);
                return ERR_MEMORY_ALLOCATION;
            }
            entry = entry->next;
        }
    }
    return COMPLETE;
}

/**
 * Map a function over the keys of the hashmap
 * 
 * @param ht Hashmap
 * @param fn Function to map
 */
void hashmap_key_map(hashmap_t ht, void (*fn)(void*))
{
    for (size_t i = 0; i < ht->capacity; i++) {
        hashmap_entry_t entry = ht->entries[i];
        while (entry) {
            fn(entry->key);
            entry = entry->next;
        }
    }
}

/**
 * Map a function over the values of the hashmap
 * 
 * @param ht Hashmap
 * @param fn Function to map
 */
void hashmap_val_map(hashmap_t ht, void (*fn)(void*))
{
    for (size_t i = 0; i < ht->capacity; i++) {
        hashmap_entry_t entry = ht->entries[i];
        while (entry) {
            fn(entry->elem);
            entry = entry->next;
        }
    }
}

/**
 * Free the entries of the hashmap
 * 
 * @param ht Hashmap
 */
void hashmap_clear(hashmap_t ht)
{
    for (size_t i = 0; i < ht->capacity; i++) {
        hashmap_entry_t entry = ht->entries[i];
        while (entry) {
            hashmap_entry_t next = entry->next;
            if (ht->free_fn) {
                ht->free_fn(entry->key);
                ht->free_fn(entry->elem);
                ht->free_fn(entry);
            } else {
                free(entry->key);
                free(entry->elem);
                free(entry);
            }
            entry = next;
        }
        ht->entries[i] = NULL;
    }
    ht->size = 0;
}

/**
 * Free the hashmap and its entries
 * 
 * @param ht Hashmap
 */
void hashmap_deinit(hashmap_t ht)
{
    hashmap_clear(ht);
    if (ht->free_fn) {
        ht->free_fn(ht->entries);
    } else {
        free(ht->entries);
    }
    free(ht);
}

/**
 * Round up to the nearest power of 2
 * See: https://graphics.stanford.edu/~seander/bithacks.html
 * 
 * @param n Number to round up
 * @return Nearest power of 2
 */
static size_t roundup_pow2(size_t n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;
    return n;
} 

uint64_t djb2hash64(const char* s, size_t len)
{
    uint64_t hash = 5381;
    while (len--)
        hash = ((hash << 5) + hash) + *s++;
    return hash;
}

/***********************************************************************************
 * Original code from cityhash-c - MIT license
 * Copyright (c) 2011-2012, Alexander Nusov
 * See: https://github.com/nusov/cityhash-c
 * 
 * Simplified version with only the CityHash64
 **********************************************************************************/

typedef struct {
    uint64_t first;
    uint64_t second;
} uint128_t;

static uint64_t fetch64(const char *p) {
    uint64_t result;
    memcpy(&result, p, sizeof(result));
    return result;
}

static uint32_t fetch32(const char *p) {
    uint32_t result;
    memcpy(&result, p, sizeof(result));
    return result;
}

static const uint64_t k0 = 0xc3a5c85c97cb3127ULL;
static const uint64_t k1 = 0xb492b66fbe98f273ULL;
static const uint64_t k2 = 0x9ae16a3b2f90404fULL;
static const uint64_t k3 = 0xc949d7c7509e6557ULL;

static inline uint64_t hash128to64(const uint128_t x)
{
    const uint64_t kmul = 0x9ddfea08eb382d69ULL;
    uint64_t a = (x.first ^ x.second) * kmul;
    a ^= (a >> 47);
    uint64_t b = (x.second ^ a) * kmul;
    b ^= (b >> 47);
    b *= kmul;
    return b;
}

static uint64_t rotate(uint64_t val, int shift)
{
    return shift == 0 ? val : ((val >> shift) | (val << (64 - shift)));
}

static uint64_t rotate1(uint64_t val, int shift)
{
    return (val >> shift) | (val << (64 - shift));
}

static uint64_t shiftmix(uint64_t val)
{
    return val ^ (val >> 47);
}

static uint64_t hashlen16(uint64_t u, uint64_t v)
{
    uint128_t result;
    result.first = u;
    result.second = v;
    return hash128to64(result);
}

static uint64_t hashlen0to16(const char *s, size_t len)
{
    if (len > 8) {
        uint64_t a = fetch64(s);
        uint64_t b = fetch64(s + len - 8);
        return hashlen16(a, rotate1(b + len, len)) ^ b;
    }
    if (len >= 4) {
        uint64_t a = fetch32(s);
        return hashlen16(len + (a << 3), fetch32(s + len - 4));
    }
    if (len > 0) {
        uint8_t a = s[0];
        uint8_t b = s[len >> 1];
        uint8_t c = s[len - 1];
        uint32_t y = (uint32_t) (a) + ((uint32_t) (b) << 8);
        uint32_t z = len + ((uint32_t) (c) << 2);
        return shiftmix(y * k2 ^ z * k3) * k2;
    }
    return k2;
}

static uint64_t hashlen17to32(const char *s, size_t len)
{
    uint64_t a = fetch64(s) * k1;
    uint64_t b = fetch64(s + 8);
    uint64_t c = fetch64(s + len - 8) * k2;
    uint64_t d = fetch64(s + len - 16) * k0;
    return hashlen16(rotate(a - b, 43) + rotate(c, 30) + d,
                     a + rotate(b ^ k3, 20) - c + len);
}

static uint128_t weakhashlen32withseeds6(uint64_t w, uint64_t x, uint64_t y,
                                         uint64_t z, uint64_t a, uint64_t b)
{
    a += w;
    b = rotate(b + a + z, 21);
    uint64_t c = a;
    a += x;
    a += y;
    b += rotate(a, 44);

    uint128_t result;
    result.first = (uint64_t) (a + z);
    result.second = (uint64_t) (b + c);
    return result;
}

static uint128_t weakhashlen32withseeds(const char *s, uint64_t a, uint64_t b)
{
    return weakhashlen32withseeds6(fetch64(s), fetch64(s + 8), fetch64(s + 16),
                                   fetch64(s + 24), a, b);
}

static uint64_t hashlen33to64(const char *s, size_t len)
{
    uint64_t z = fetch64(s + 24);
    uint64_t a = fetch64(s) + (len + fetch64(s + len - 16)) * k0;
    uint64_t b = rotate(a + z, 52);
    uint64_t c = rotate(a, 37);
    a += fetch64(s + 8);
    c += rotate(a, 7);
    a += fetch64(s + 16);
    uint64_t vf = a + z;
    uint64_t vs = b + rotate(a, 31) + c;
    a = fetch64(s + 16) + fetch64(s + len - 32);
    z = fetch64(s + len - 8);
    b = rotate(a + z, 52);
    c = rotate(a, 37);
    a += fetch64(s + len - 24);
    c += rotate(a, 7);
    a += fetch64(s + len - 16);
    uint64_t wf = a + z;
    uint64_t ws = b + rotate(a, 31) + c;
    uint64_t r = shiftmix((vf + ws) * k2 + (wf + vs) * k0);
    return shiftmix(r * k0 + vs) * k2;
}

uint64_t cityhash64(const char *s, size_t len)
{
    if (len <= 32) {
        if (len <= 16) {
            return hashlen0to16(s, len);
        } else {
            return hashlen17to32(s, len);
        }
    } else if (len <= 64) {
        return hashlen33to64(s, len);
    }
    uint64_t x = fetch64(s + len - 40);
    uint64_t y = fetch64(s + len - 16) + fetch64(s + len - 56);
    uint64_t z = hashlen16(fetch64(s + len - 48) + len, fetch64(s + len - 24));
    uint64_t temp;
    uint128_t v = weakhashlen32withseeds(s + len - 64, len, z);
    uint128_t w = weakhashlen32withseeds(s + len - 32, y + k1, x);
    x = x * k1 + fetch64(s);

    len = (len - 1) & ~(size_t) (63);
    do {
	    x = rotate(x + y + v.first + fetch64(s + 8), 37) * k1;
	    y = rotate(y + v.second + fetch64(s + 48), 42) * k1;
	    x ^= w.second;
	    y += v.first + fetch64(s + 40);
	    z = rotate(z + w.first, 33) * k1;
	    v = weakhashlen32withseeds(s, v.second * k1, x + w.first);
	    w = weakhashlen32withseeds(s + 32, z + w.second,
				                   y + fetch64(s + 16));
	    temp = z;
	    z = x;
	    x = temp;
	    s += 64;
	    len -= 64;
    } while (len != 0);
    return hashlen16(hashlen16(v.first, w.first) + shiftmix(y) * k1 + z,
                     hashlen16(v.second, w.second) + x);
}
