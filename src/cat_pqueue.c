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

#include "cat_pqueue.h"

#include <stdlib.h>
#include <string.h>

#define PQUEUE_EXP_FACTOR 2

#define pqueue_shift(pq, i) ((char*)pq->heap + (i) * pq->elem_size)
#define parent(i) ((i - 1) / 2)
#define lchild(i) (2 * i + 1)
#define rchild(i) (2 * i + 2)

typedef struct pqueue_s {
    size_t      size;
    size_t      capacity;
    size_t      elem_size;
    void       *heap;

    int       (*cmp_fn)(const void*, const void*);
    void      (*free_fn)(void*);
    void     *(*alloc_fn)(size_t);
} pqueue_s;

static stat_t pqueue_alloc(pqueue_t pq, size_t capacity);

static void heapify_down(pqueue_t pq, size_t i);
static void heapify_up(pqueue_t pq, size_t i);

/**
 * Get the size of the priority queue
 * 
 * @param pq Priority queue
 * @return Size of the priority queue
 */
size_t pqueue_size(pqueue_t pq)
{
    return pq->size;
}

/**
 * Get the capacity of the priority queue
 * 
 * @param pq Priority queue
 * @return Capacity of the priority queue
 */
size_t pqueue_capacity(pqueue_t pq)
{
    return pq->capacity;
}

/**
 * Check if the priority queue is empty
 * 
 * @param pq Priority queue
 * @return 1 if the priority queue is empty, 0 otherwise
 */
int pqueue_is_empty(pqueue_t pq)
{
    return pq->size == 0;
}

/**
 * Check if the priority queue is full
 * 
 * @param pq Priority queue
 * @return 1 if the priority queue is full, 0 otherwise
 */
int pqueue_is_full(pqueue_t pq)
{
    return pq->size >= pq->capacity;
}

/**
 * Check if an element is in the priority queue
 * 
 * @param pq Priority queue
 * @param elem Pointer to the element to count
 * @return Number of the element in the priority queue
 */
size_t pqueue_contains(pqueue_t pq, void* elem)
{
    size_t count = 0;
    for (size_t i = 0; i < pq->size; i++) {
        if (pq->cmp_fn(pqueue_shift(pq, i), elem) == 0)
            count++;
    }
    return count;
}

/**
 * Allocate memory for the priority queue
 * 
 * @param pq Priority queue
 * @param capacity Capacity of the buffer to allocate
 * @return COMPLETE on success, corresponding error code on failure
 */
static stat_t pqueue_alloc(pqueue_t pq, size_t capacity)
{
    void* buffer = NULL;
    void* (*alloc)(size_t) = pq->alloc_fn ? pq->alloc_fn : malloc;
    void (*dealloc)(void*) = pq->free_fn ? pq->free_fn : free;

    if (pq->heap) {
        if (alloc == malloc && dealloc == free) {
            buffer = realloc(pq->heap, capacity * pq->elem_size);
            if (!buffer) return ERR_MEMORY_ALLOCATION;
            pq->heap = buffer;
            return COMPLETE;
        }
        buffer = alloc(capacity * pq->elem_size);
        if (!buffer) return ERR_MEMORY_ALLOCATION;

        memcpy(buffer, pq->heap, pq->size * pq->elem_size);
        dealloc(pq->heap);
    } else {
        buffer = alloc(capacity * pq->elem_size);
        if (!buffer) return ERR_MEMORY_ALLOCATION;
    }
    pq->heap = buffer;
    return COMPLETE;
}

/**
 * Initialize a priority queue
 * 
 * @param capacity Initial capacity of the priority queue
 * @param elem_size Size of each element in the priority queue
 * @param cmp_fn Comparison function
 * @param free_fn Free function, NULL for default free
 * @param alloc_fn Alloc function, NULL for default malloc
 * @return Initialized priority queue on success, NULL on failure
 */
pqueue_t _pqueue_init(size_t capacity,
                      size_t elem_size,
                      int (*cmp_fn)(const void*, const void*),
                      void (*free_fn)(void*),
                      void* (*alloc_fn)(size_t))
{
    if (capacity > ((size_t)0 - 1) / elem_size)
        return NULL;
    pqueue_t pq = (pqueue_t)malloc(sizeof(pqueue_s));
    if (!pq) return NULL;

    pq->size = 0;
    pq->capacity = capacity;
    pq->elem_size = elem_size;
    pq->heap = NULL;

    pq->cmp_fn = cmp_fn;
    pq->free_fn = free_fn;
    pq->alloc_fn = alloc_fn;

    // last position is used as temp for swap
    if (pqueue_alloc(pq, capacity + 1)) {
        free(pq);
        return NULL;
    }
    return pq;
}

/**
 * Reserve memory for the priority queue
 * 
 * @param pq Priority queue
 * @param capacity Capacity of the buffer to reserve
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t pqueue_reserve(pqueue_t pq, size_t capacity)
{
    if (capacity <= pq->capacity ||
        capacity >= ((size_t)0 - 1) / pq->elem_size)
        return ERR_INVALID_OPERATION;

    if (pqueue_alloc(pq, capacity + 1))
        return ERR_MEMORY_ALLOCATION;
    pq->capacity = capacity;
    return COMPLETE;
}

/**
 * Shrink the priority queue capacity to fit the size
 * 
 * @param pq Priority queue
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t pqueue_shrink_to_fit(pqueue_t pq)
{
    if (pq->size == 0) return ERR_INVALID_OPERATION;
    if (pq->capacity == pq->size) return COMPLETE;

    if (pqueue_alloc(pq, pq->size + 1))
        return ERR_MEMORY_ALLOCATION;
    pq->capacity = pq->size;
    return COMPLETE;
}

/**
 * Heapify down an element in the priority queue
 * 
 * @param pq Priority queue
 * @param i Index of the element to heapify down
 */
static void heapify_down(pqueue_t pq, size_t i)
{
    while (1) {
        size_t l = lchild(i);
        size_t r = rchild(i);
        size_t m = i;

        if (l < pq->size &&
            pq->cmp_fn(pqueue_shift(pq, l),
                       pqueue_shift(pq, m)) < 0)
            m = l;
        if (r < pq->size &&
            pq->cmp_fn(pqueue_shift(pq, r),
                       pqueue_shift(pq, m)) < 0)
            m = r;
        if (m == i) break;
        memcpy(pqueue_shift(pq, pq->capacity),
               pqueue_shift(pq, m),
               pq->elem_size);
        memcpy(pqueue_shift(pq, m),
               pqueue_shift(pq, i),
               pq->elem_size);
        memcpy(pqueue_shift(pq, i),
               pqueue_shift(pq, pq->capacity),
               pq->elem_size);
        i = m;
    }
}

/**
 * Heapify up an element in the priority queue
 * 
 * @param pq Priority queue
 * @param i Index of the element to heapify up
 */
static void heapify_up(pqueue_t pq, size_t i)
{
    while (i > 0) {
        size_t p = parent(i);
        if (pq->cmp_fn(pqueue_shift(pq, i),
                       pqueue_shift(pq, p)) >= 0)
            break;
        memcpy(pqueue_shift(pq, pq->capacity),
               pqueue_shift(pq, i),
               pq->elem_size);
        memcpy(pqueue_shift(pq, i),
               pqueue_shift(pq, p),
               pq->elem_size);
        memcpy(pqueue_shift(pq, p),
               pqueue_shift(pq, pq->capacity),
               pq->elem_size);
        i = p;
    }
}

/**
 * Push an element into the priority queue
 * 
 * @param pq Priority queue
 * @param elem Element to push
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t pqueue_push(pqueue_t pq, void* elem)
{
    if (pq->size >= pq->capacity) {
        if (pq->capacity >= ((size_t)0 - 2) / pq->elem_size / PQUEUE_EXP_FACTOR)
            return ERR_CAPACITY_OVERFLOW;
        if (pqueue_alloc(pq, pq->capacity * PQUEUE_EXP_FACTOR + 1))
            return ERR_MEMORY_ALLOCATION;
        pq->capacity *= PQUEUE_EXP_FACTOR;
    }

    memcpy(pqueue_shift(pq, pq->size),
           elem,
           pq->elem_size);
    heapify_up(pq, pq->size++);
    return COMPLETE;
}

/**
 * Pop an element from the priority queue
 * 
 * @param pq Priority queue
 * @param elem Pointer to the element to pop, NULL to discard
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t pqueue_pop(pqueue_t pq, void* ret_elem)
{
    if (pq->size == 0) return ERR_INVALID_OPERATION;

    if (ret_elem)
        memcpy(ret_elem, pqueue_shift(pq, 0), pq->elem_size);

    if (pq->size > 1) {
        memcpy(pqueue_shift(pq, 0),
               pqueue_shift(pq, pq->size - 1),
               pq->elem_size);
        heapify_down(pq, 0);
    }
    pq->size--;
    return COMPLETE;
}

/**
 * Get the top element of the priority queue
 * 
 * @param pq Priority queue
 * @param elem Pointer to the element to get
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t pqueue_top(pqueue_t pq, void* elem)
{
    if (pq->size == 0) return ERR_INVALID_OPERATION;

    memcpy(elem, pqueue_shift(pq, 0), pq->elem_size);
    return COMPLETE;
}

/**
 * Copy a priority queue with copy of the elements
 * 
 * @param dst Pointer to the destination priority queue
 * @param src Source priority queue
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t pqueue_copy(pqueue_t* dst, pqueue_t src)
{
    *dst = (pqueue_t)malloc(sizeof(pqueue_s));
    if (!*dst) return ERR_MEMORY_ALLOCATION;

    (*dst)->size = src->size;
    (*dst)->capacity = src->capacity;
    (*dst)->elem_size = src->elem_size;
    (*dst)->heap = NULL;

    (*dst)->cmp_fn = src->cmp_fn;
    (*dst)->free_fn = src->free_fn;
    (*dst)->alloc_fn = src->alloc_fn;

    if (pqueue_alloc(*dst, src->capacity + 1)) {
        free(*dst);
        return ERR_MEMORY_ALLOCATION;
    }
    memcpy((*dst)->heap, src->heap, src->size * src->elem_size);

    return COMPLETE;
}

/**
 * Map a function over the priority queue
 * 
 * @param pq Priority queue
 * @param fn Function to map over the priority queue
 */
void pqueue_map(pqueue_t pq, void (*fn)(void*))
{
    for (size_t i = 0; i < pq->size; i++) {
        fn(pqueue_shift(pq, i));
    }
}

/**
 * Clear the priority queue
 * 
 * @param pq Priority queue
 */
void pqueue_clear(pqueue_t pq)
{
    pq->size = 0;
}

/**
 * Destroy the priority queue
 * 
 * @param pq Priority queue
 */
void pqueue_deinit(pqueue_t pq)
{
    if (pq->free_fn) {
        pq->free_fn(pq->heap);
    } else {
        free(pq->heap);
    }
    free(pq);
}
