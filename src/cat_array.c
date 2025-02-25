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

#include "cat_array.h"

#include <stdlib.h>
#include <string.h>

#define ARRAY_EXP_FACTOR 2

#define array_shift(arr, i) ((char*)arr->array + (i) * arr->elem_size)

typedef struct array_s {
    size_t          size;
    size_t          capacity;
    size_t          elem_size;
    void           *array;

    void          (*free_fn)(void*);
    void         *(*alloc_fn)(size_t);
} array_s;

static stat_t array_alloc(array_t arr, size_t capacity);

/**
 * Get the size of the array
 * 
 * @param arr Array
 * @return Size of the array
 */
size_t array_size(array_t arr)
{
    return arr->size;
}

/**
 * Get the capacity of the array
 * 
 * @param arr Array
 * @return Capacity of the array
 */
size_t array_capacity(array_t arr)
{
    return arr->capacity;
}

/**
 * Get the buffer of the array
 * 
 * @param arr Array
 * @return Internal buffer of the array
 */
void* array_data(array_t arr)
{
    return (void* const)(arr->array);
}

/**
 * Check if the array is empty
 * 
 * @param arr Array
 * @return 1 if the array is empty, 0 otherwise
 */
int array_is_empty(array_t arr)
{
    return arr->size == 0;
}

/**
 * Check if the array is full
 * 
 * @param arr Array
 * @return 1 if the array is full, 0 otherwise
 */
int array_is_full(array_t arr)
{
    return arr->size >= arr->capacity;
}

/**
 * Check if an element is in the array
 * 
 * @param arr Array
 * @param elem Pointer to the element to count
 * @param cmp_fn Comparison function, NULL for default memcmp
 * @return Number of the element in the array
 */
size_t array_contains(array_t arr,
                      void* elem,
                      int (*cmp_fn)(const void*, const void*))
{
    size_t count = 0;
    for (size_t i = 0; i < arr->size; i++) {
        int match = cmp_fn ? 
            cmp_fn(array_shift(arr, i), elem) :
            memcmp(array_shift(arr, i), elem, arr->elem_size);
        if (match == 0) count++;
    }
    return count;
}

/**
 * Allocate or reallocate memory for the array
 * 
 * @param arr Array
 * @param capacity Capacity of the buffer to allocate
 * @return COMPLETE on success, corresponding error code on failure
 */
static stat_t array_alloc(array_t arr, size_t capacity)
{
    void* buffer = NULL;
    void* (*alloc)(size_t) = arr->alloc_fn ? arr->alloc_fn : malloc;
    void (*dealloc)(void*) = arr->free_fn ? arr->free_fn : free;

    if (arr->array) {
        if (alloc == malloc && dealloc == free) {
            buffer = realloc(arr->array, capacity * arr->elem_size);
            if (!buffer) return ERR_MEMORY_ALLOCATION;
            arr->array = buffer;
            return COMPLETE;
        }
        buffer = alloc(capacity * arr->elem_size);
        if (!buffer) return ERR_MEMORY_ALLOCATION;
        
        memcpy(buffer, arr->array, arr->size * arr->elem_size);
        dealloc(arr->array);
    } else {
        buffer = alloc(capacity * arr->elem_size);
        if (!buffer) return ERR_MEMORY_ALLOCATION;
    }
    
    arr->array = buffer;
    return COMPLETE;
}

/**
 * Initialize an array
 * 
 * @param capacity Initial capacity of the array
 * @param elem_size Size of each element in the array
 * @param free_fn Free function, NULL for default free
 * @param alloc_fn Alloc function, NULL for default malloc
 * @return Initialized array on success, NULL on failure
 */
array_t _array_init(size_t capacity,
                    size_t elem_size,
                    void (*free_fn)(void*),
                    void* (*alloc_fn)(size_t))
{
    if (capacity >= ((size_t)0 - 1) / elem_size)
        return NULL;
    array_t arr = (array_t)malloc(sizeof(array_s));
    if (!arr) return NULL;

    arr->capacity = capacity;
    arr->elem_size = elem_size;
    arr->size = 0;
    arr->array = NULL;

    arr->free_fn = free_fn;
    arr->alloc_fn = alloc_fn;

    if (array_alloc(arr, capacity)) {
        free(arr);
        return NULL;
    }
    return arr;
}

/**
 * Reserve memory for the array
 * 
 * @param arr Array
 * @param capacity Capacity of the buffer to reserve
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_reserve(array_t arr, size_t capacity)
{
    if (capacity <= arr->capacity ||
        capacity >= ((size_t)0 - 1) / arr->elem_size)
        return ERR_INVALID_OPERATION;

    if (array_alloc(arr, capacity))
        return ERR_MEMORY_ALLOCATION;
    arr->capacity = capacity;
    return COMPLETE;
}

/**
 * Shrink the array capacity to fit the size
 * 
 * @param arr Array
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_shrink_to_fit(array_t arr)
{
    if (arr->size == 0) return ERR_INVALID_OPERATION;
    if (arr->capacity == arr->size) return COMPLETE;

    if (array_alloc(arr, arr->size))
        return ERR_MEMORY_ALLOCATION;
    arr->capacity = arr->size;
    return COMPLETE;
}

/**
 * Push an element to the end of the array
 * 
 * @param arr Array
 * @param elem Pointer to the element to push
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_push_back(array_t arr, void* elem)
{
    if (arr->size >= arr->capacity) {
        if (arr->capacity >= ((size_t)0 - 1) / arr->elem_size / ARRAY_EXP_FACTOR)
            return ERR_CAPACITY_OVERFLOW;
        if (array_alloc(arr, arr->capacity * ARRAY_EXP_FACTOR))
            return ERR_MEMORY_ALLOCATION;
        arr->capacity *= ARRAY_EXP_FACTOR;
    }

    memcpy(array_shift(arr, arr->size), elem, arr->elem_size);
    arr->size++;

    return COMPLETE;
}

/**
 * Pop an element from the end of the array
 * 
 * @param arr Array
 * @param ret_elem Pointer to the element to pop, NULL to discard
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_pop_back(array_t arr, void* ret_elem)
{
    if (arr->size == 0) return ERR_INVALID_OPERATION;

    arr->size--;
    if (ret_elem)
        memcpy(ret_elem, array_shift(arr, arr->size), arr->elem_size);

    return COMPLETE;
}

/**
 * Insert an element into the array at a specific index
 * 
 * @param arr Array
 * @param elem Pointer to the element to insert
 * @param i Index at which to insert the element
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_insert(array_t arr, void* elem, size_t i)
{
    if (i > arr->size) return ERR_INDEX_OUT_OF_RANGE;
    if (arr->size >= arr->capacity) {
        if (arr->capacity >= ((size_t)0 - 1) / arr->elem_size / ARRAY_EXP_FACTOR)
            return ERR_CAPACITY_OVERFLOW;
        if (array_alloc(arr, arr->capacity * ARRAY_EXP_FACTOR))
            return ERR_MEMORY_ALLOCATION;
        arr->capacity *= ARRAY_EXP_FACTOR;
    }

    if (i < arr->size) {
        memmove(array_shift(arr, i + 1),
        array_shift(arr, i),
        (arr->size - i) * arr->elem_size);
    }
    memcpy(array_shift(arr, i), elem, arr->elem_size);
    arr->size++;

    return COMPLETE;
}

/**
 * Remove an element from the array at a specific index
 * 
 * @param arr Array
 * @param ret_elem Pointer to the element to remove, NULL to discard
 * @param i Index at which to remove the element
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_remove(array_t arr, void* ret_elem, size_t i)
{
    if (arr->size == 0) return ERR_INVALID_OPERATION;
    if (i >= arr->size) return ERR_INDEX_OUT_OF_RANGE;

    if (ret_elem)
        memcpy(ret_elem, array_shift(arr, i), arr->elem_size);

    if (i != arr->size - 1) {
        memmove(array_shift(arr, i),
        array_shift(arr, i + 1),
        (arr->size - i - 1) * arr->elem_size);
    }
    arr->size--;

    return COMPLETE;
}

/**
 * Get the first element of the array
 * 
 * @param arr Array
 * @param ret_elem Pointer to the element to get
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_front(array_t arr, void* ret_elem)
{
    return array_get(arr, ret_elem, 0);
}

/**
 * Get the last element of the array
 * 
 * @param arr Array
 * @param ret_elem Pointer to the element to get
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_back(array_t arr, void* ret_elem)
{
    return array_get(arr, ret_elem, arr->size - 1);
}

/**
 * Get an element from the array at a specific index
 * 
 * @param arr Array
 * @param ret_elem Pointer to the element to get
 * @param i Index at which to get the element
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_get(array_t arr, void* ret_elem, size_t i)
{
    if (i >= arr->size) return ERR_INDEX_OUT_OF_RANGE;

    memcpy(ret_elem, array_shift(arr, i), arr->elem_size);
    return COMPLETE;
}

/**
 * Set an element in the array at a specific index
 * 
 * @param arr Array
 * @param elem Pointer to the element to set
 * @param i Index at which to set the element
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_set(array_t arr, void* elem, size_t i)
{
    if (i >= arr->size) return ERR_INDEX_OUT_OF_RANGE;

    memcpy(array_shift(arr, i), elem, arr->elem_size);
    return COMPLETE;
}

/**
 * Copy an array with copy of the elements
 * 
 * @param dst Pointer to the destination
 * @param src Source
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t array_copy(array_t* dst, array_t src)
{
    *dst = (array_t)malloc(sizeof(array_s));
    if (!*dst) return ERR_MEMORY_ALLOCATION;

    (*dst)->capacity = src->capacity;
    (*dst)->elem_size = src->elem_size;
    (*dst)->size = src->size;
    (*dst)->array = NULL;

    (*dst)->free_fn = src->free_fn;
    (*dst)->alloc_fn = src->alloc_fn;

    if (array_alloc(*dst, src->capacity)) {
        free(*dst);
        return ERR_MEMORY_ALLOCATION;
    }
    memcpy((*dst)->array, src->array, src->size * src->elem_size);

    return COMPLETE;
}

/**
 * Get an element from the array at a specific index
 * 
 * @param arr Array
 * @param i Index at which to get the element
 * @return Pointer to the element
 */
void* array_at(array_t arr, size_t i)
{
    if (i >= arr->size) return NULL;
    return array_shift(arr, i);
}

/**
 * Locate an element in the array using binary search
 * 
 * @param arr Array
 * @param elem Pointer to the element to locate
 * @param cmp_fn Comparison function
 * @return Pointer to the element if found, NULL otherwise
 */
void* array_bsearch(array_t arr,
                    void* elem,
                    int (*cmp_fn)(const void*, const void*))
{
    return bsearch(elem, arr->array, arr->size, arr->elem_size, cmp_fn);
}

/**
 * Sort the array using qsort
 * 
 * @param arr Array
 * @param cmp_fn Comparison function
 */
void array_qsort(array_t arr,
                 int (*cmp_fn)(const void*, const void*))
{
    qsort(arr->array, arr->size, arr->elem_size, cmp_fn);
}

/**
 * Map a function over the array
 * 
 * @param arr Array
 * @param fn Function to map over the array
 */
void array_map(array_t arr, void (*fn)(void*))
{
    for (size_t i = 0; i < arr->size; i++) {
        fn(array_shift(arr, i));
    }
}

/**
 * Clear the array without freeing the elements
 * 
 * @param arr Array
 */
void array_clear(array_t arr)
{
    arr->size = 0;
}

/**
 * Free the array and its elements
 * 
 * @param arr Array
 */
void array_deinit(array_t arr)
{
    if (arr->free_fn) {
        arr->free_fn(arr->array);
    } else {
        free(arr->array);
    }
    free(arr);
}
