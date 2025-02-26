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

#include "cat_deque.h"

#include <stdlib.h>
#include <string.h>

#define DEQUE_EXP_FACTOR 2
#define DEQUE_DEFAULT_CAPACITY 8

#define deque_shift(deq, i) ((char*)deq->deque + (i) * deq->elem_size)

typedef struct deque_s {
    size_t          front;
    size_t          rear;
    size_t          size;
    size_t          capacity;
    size_t          elem_size;
    void           *deque;

    void          (*free_fn)(void*);
    void         *(*alloc_fn)(size_t);
} deque_s;

static stat_t deque_alloc(deque_t deq, size_t capacity);

/**
 * Get the size of the deque
 * 
 * @param deq Deque
 * @return Size of the deque
 */
size_t deque_size(deque_t deq)
{
    return deq->size;
}

/**
 * Get the capacity of the deque
 * 
 * @param deq Deque
 * @return Capacity of the deque
 */
size_t deque_capacity(deque_t deq)
{
    return deq->capacity;
}

/**
 * Check if the deque is empty
 * 
 * @param deq Deque
 * @return 1 if the deque is empty, 0 otherwise
 */
int deque_is_empty(deque_t deq)
{
    return deq->size == 0;
}

/**
 * Check if the deque is full
 * 
 * @param deq Deque
 * @return 1 if the deque is full, 0 otherwise
 */
int deque_is_full(deque_t deq)
{
    return deq->size >= deq->capacity;
}

/**
 * Check if an element is in the deque
 * 
 * @param deq Deque
 * @param elem Pointer to the element to count
 * @param cmp_fn Comparison function, NULL for default memcmp
 * @return Number of the element in the deque
 */
size_t deque_contains(deque_t deq,
                      void* elem,
                      int (*cmp_fn)(const void*, const void*))
{
    size_t count = 0;
    for (size_t j = 0, i = deq->front; j < deq->size; j++) {
        int match = cmp_fn ? 
            cmp_fn(deque_shift(deq, i), elem) :
            memcmp(deque_shift(deq, i), elem, deq->elem_size);
        if (match == 0) count++;
        i = (i + 1) % deq->capacity;
    } 
    return count;
}

/**
 * Allocate memory for the deque
 * 
 * @param deq Deque
 * @param size Capacity of the buffer to allocate
 * @return Status of the allocation
 */
static stat_t deque_alloc(deque_t deq, size_t capacity)
{
    void* buffer = NULL;
    void* (*alloc)(size_t) = deq->alloc_fn ? deq->alloc_fn : malloc;
    void (*dealloc)(void*) = deq->free_fn ? deq->free_fn : free;

    buffer = alloc(capacity * deq->elem_size);
    if (!buffer) return ERR_MEMORY_ALLOCATION;

    if (deq->deque) {
        for (size_t i = 0, j = deq->front; i < deq->size; i++) {
            memcpy((char*)buffer + i * deq->elem_size,
                   deque_shift(deq, j),
                   deq->elem_size);
            j = (j == deq->capacity - 1) ? 0 : j + 1;
        }
        dealloc(deq->deque);
    }
    deq->deque = buffer;
    deq->front = 0;
    deq->rear = deq->size;
    return COMPLETE; 
}

/**
 * Initialize the deque
 * 
 * @param capacity Initial capacity of the deque
 * @param elem_size Size of each element in the deque
 * @param free_fn Free function, NULL for default free
 * @param alloc_fn Alloc function, NULL for default malloc
 * @return Initialized deque on success, NULL on failure
 */
deque_t _deque_init(size_t capacity,
                    size_t elem_size,
                    void (*free_fn)(void*),
                    void* (*alloc_fn)(size_t))
{
    if (capacity >= ((size_t)0 - 1) / elem_size)
        return NULL;
    deque_t deq = (deque_t)malloc(sizeof(deque_s));
    if (!deq) return NULL;

    deq->front = 0;
    deq->rear = 0;
    deq->size = 0;
    deq->capacity = capacity ? capacity : DEQUE_DEFAULT_CAPACITY;
    deq->elem_size = elem_size;
    deq->deque = NULL;

    deq->free_fn = free_fn;
    deq->alloc_fn = alloc_fn;

    if (deque_alloc(deq, capacity)) {
        free(deq);
        return NULL;
    }
    return deq;
}

/**
 * Reserve memory for the deque
 * 
 * @param deq Deque
 * @param capacity Capacity of the buffer to reserve
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_reserve(deque_t deq, size_t capacity)
{
    if (capacity <= deq->capacity ||
        capacity >= ((size_t)0 - 1) / deq->elem_size)
        return ERR_INVALID_OPERATION;

    if (deque_alloc(deq, capacity))
        return ERR_MEMORY_ALLOCATION;
    deq->capacity = capacity;
    return COMPLETE;
}

/**
 * Shrink the deque capacity to fit the size
 * 
 * @param deq Deque
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_shrink_to_fit(deque_t deq)
{
    if (deq->size == 0) return ERR_INVALID_OPERATION;
    if (deq->capacity == deq->size) return COMPLETE;

    if (deque_alloc(deq, deq->size))
        return ERR_MEMORY_ALLOCATION;
    deq->capacity = deq->size;
    return COMPLETE;
}

/**
 * Push an element to the front of the deque
 * 
 * @param deq Deque
 * @param elem Pointer to the element to push
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_push_front(deque_t deq, void* elem)
{
    if (deq->size >= deq->capacity) {
        if (deq->capacity >= ((size_t)0 - 1) / deq->elem_size / DEQUE_EXP_FACTOR)
            return ERR_CAPACITY_OVERFLOW;
        if (deque_alloc(deq, deq->capacity * DEQUE_EXP_FACTOR))
            return ERR_MEMORY_ALLOCATION;
        deq->capacity *= DEQUE_EXP_FACTOR;
    }

    deq->front = (deq->front == 0) ? deq->capacity - 1 : deq->front - 1;
    memcpy(deque_shift(deq, deq->front), elem, deq->elem_size);
    deq->size++;

    return COMPLETE;
}

/**
 * Push an element to the back of the deque
 * 
 * @param deq Deque
 * @param elem Pointer to the element to push
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_push_back(deque_t deq, void* elem)
{
    if (deq->size >= deq->capacity) {
        if (deq->capacity >= ((size_t)0 - 1) / deq->elem_size / DEQUE_EXP_FACTOR)
            return ERR_CAPACITY_OVERFLOW;
        if (deque_alloc(deq, deq->capacity * DEQUE_EXP_FACTOR))
            return ERR_MEMORY_ALLOCATION;
        deq->capacity *= DEQUE_EXP_FACTOR;
    }

    memcpy(deque_shift(deq, deq->rear), elem, deq->elem_size);
    deq->rear = (deq->rear == deq->capacity - 1) ? 0 : deq->rear + 1;
    deq->size++;

    return COMPLETE;
}

/**
 * Pop an element from the front of the deque
 * 
 * @param deq Deque
 * @param ret_elem Pointer to the element to pop, NULL to discard
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_pop_front(deque_t deq, void* ret_elem)
{
    if (deq->size == 0) return ERR_INVALID_OPERATION;

    if (ret_elem)
        memcpy(ret_elem, deque_shift(deq, deq->front), deq->elem_size);
    deq->front = (deq->front == deq->capacity - 1) ? 0 : deq->front + 1;
    deq->size--;

    return COMPLETE;
}

/**
 * Pop an element from the back of the deque
 * 
 * @param deq Deque
 * @param ret_elem Pointer to the element to pop, NULL to discard
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_pop_back(deque_t deq, void* ret_elem)
{
    if (deq->size == 0) return ERR_INVALID_OPERATION;

    deq->rear = (deq->rear == 0) ? deq->capacity - 1 : deq->rear - 1;
    if (ret_elem)
        memcpy(ret_elem, deque_shift(deq, deq->rear), deq->elem_size);
    deq->size--;

    return COMPLETE;
}

/**
 * Get the front element of the deque
 * 
 * @param deq Deque
 * @param ret_elem Pointer to the element to get
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_front(deque_t deq, void* ret_elem)
{
    if (deq->size == 0) return ERR_INVALID_OPERATION;

    memcpy(ret_elem, deque_shift(deq, deq->front), deq->elem_size);
    return COMPLETE;
}

/**
 * Get the back element of the deque
 * 
 * @param deq Deque
 * @param ret_elem Pointer to the element to get
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_back(deque_t deq, void* ret_elem)
{
    if (deq->size == 0) return ERR_INVALID_OPERATION;

    memcpy(ret_elem,
           deque_shift(deq, (deq->rear == 0) ?
                             deq->capacity - 1 :
                             deq->rear - 1),
           deq->elem_size);
    return COMPLETE;
}

/**
 * Get an element from the deque at a specific index
 * 
 * @param deq Deque
 * @param ret_elem Pointer to the element to get
 * @param i Index of the element to get
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_get(deque_t deq, void* ret_elem, size_t i)
{
    if (i >= deq->size) return ERR_INDEX_OUT_OF_RANGE;

    size_t j = (deq->front + i) % deq->capacity;
    memcpy(ret_elem, deque_shift(deq, j), deq->elem_size);
    return COMPLETE;
}

/**
 * Set an element in the deque at a specific index
 * 
 * @param deq Deque
 * @param elem Pointer to the element to set
 * @param i Index of the element to set
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_set(deque_t deq, void* elem, size_t i)
{
    if (i >= deq->size) return ERR_INDEX_OUT_OF_RANGE;

    size_t j = (deq->front + i) % deq->capacity;
    memcpy(deque_shift(deq, j), elem, deq->elem_size);
    return COMPLETE;
}

/**
 * Concatenate two deques
 * 
 * @param dst Destination deque
 * @param src Source deque
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_concat(deque_t dst, deque_t src)
{
    if (src->size == 0) return COMPLETE;
    if (src->size > (((size_t)0 - 1) / dst->elem_size) - dst->size)
        return ERR_CAPACITY_OVERFLOW;
    if (dst->size + src->size > dst->capacity) {
        if (deque_alloc(dst, dst->size + src->size))
            return ERR_MEMORY_ALLOCATION;
        dst->capacity = dst->size + src->size;
    }
    for (size_t i = dst->rear, j = src->front, k = 0; k < src->size; k++) {
        memcpy(deque_shift(dst, i), deque_shift(src, j), dst->elem_size);
        i = (i == dst->capacity - 1) ? 0 : i + 1;
        j = (j == src->capacity - 1) ? 0 : j + 1;
    }
    dst->rear = (dst->rear + src->size) % dst->capacity;
    dst->size += src->size;
    return COMPLETE;
}

/**
 * Copy a deque with copy of the elements
 * 
 * @param dst Pointer to the destination
 * @param src Source
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t deque_copy(deque_t* dst, deque_t src)
{
    *dst = (deque_t)malloc(sizeof(deque_s));
    if (!*dst) return ERR_MEMORY_ALLOCATION;

    (*dst)->front = src->front;
    (*dst)->rear = src->rear;
    (*dst)->size = src->size;
    (*dst)->capacity = src->capacity;
    (*dst)->elem_size = src->elem_size;
    (*dst)->deque = NULL;

    (*dst)->free_fn = src->free_fn;
    (*dst)->alloc_fn = src->alloc_fn;

    if (deque_alloc(*dst, src->capacity)) {
        free(*dst);
        return ERR_MEMORY_ALLOCATION;
    }
    memcpy((*dst)->deque, src->deque, src->capacity * src->elem_size);

    return COMPLETE;
}

/**
 * Get an element from the deque at a specific index
 * 
 * @param deq Deque
 * @param i Index at which to get the element
 * @return Pointer to the element
 */
void* deque_at(deque_t deq, size_t i)
{
    if (i >= deq->size) return NULL;
    return deque_shift(deq, (deq->front + i) % deq->capacity);
}

/**
 * Map a function over the deque
 * 
 * @param deq Deque
 * @param fn Function to map over the deque
 */
void deque_map(deque_t deq, void (*fn)(void*))
{
    for (size_t i = 0, j = deq->front; i < deq->size; i++) {
        fn(deque_shift(deq, j));
        j = (j == deq->capacity - 1) ? 0 : j + 1;
    }
}

/**
 * Clear the deque without freeing the elements
 * 
 * @param deq Deque
 */
void deque_clear(deque_t deq)
{
    deq->front = deq->rear = 0;
    deq->size = 0;
}

/**
 * Free the deque and its elements
 * 
 * @param deq Deque
 */
void deque_deinit(deque_t deq)
{
    if (deq->free_fn) {
        deq->free_fn(deq->deque);
    } else {
        free(deq->deque);
    }
    free(deq);
}
