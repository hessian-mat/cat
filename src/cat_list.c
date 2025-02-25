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

#include "cat_list.h"

#include <stdlib.h>
#include <string.h>

typedef struct list_s {
    struct node_s  *head;
    struct node_s  *tail;
    size_t          size;
    size_t          elem_size;

    void          (*free_fn)(void*);
    void         *(*alloc_fn)(size_t);
} list_s;

typedef struct node_s {
    struct node_s *next;
    struct node_s *prev;
    void          *elem;
} node_s, *node_t;

typedef struct list_it_s {
    struct list_s *list;
    struct node_s *current;
} list_it_s;

static stat_t node_alloc(list_t list, node_t* node);

static void node_unlink(list_t list, node_t node);
static void node_link(list_t list, node_t node, node_t link);

static void list_get_node(list_t list, node_t* ret_node, size_t i);

/**
 * Get the size of the list
 * 
 * @param list List
 * @return Size of the list
 */
size_t list_size(list_t list)
{
    return list->size;
}

/**
 * Check if the list is empty
 * 
 * @param list List
 * @return 1 if the list is empty, 0 otherwise
 */
int list_is_empty(list_t list)
{
    return list->size == 0;
}

/**
 * Check if an element is in the list
 * 
 * @param list List
 * @param elem Pointer to the element to count
 * @param cmp_fn Comparison function, NULL for memcmp
 * @return Number of the element in the list
 */
size_t list_contains(list_t list,
                     void* elem,
                     int (*cmp_fn)(const void*, const void*))
{
    node_t node = list->head;
    size_t count = 0;
    while (node) {
        int match = cmp_fn ? 
            cmp_fn(node->elem, elem) :
            memcmp(node->elem, elem, list->elem_size);
        if (match == 0) count++;
        node = node->next;
    }
    return count;
}

/**
 * Allocate memory for a node
 * 
 * @param list List
 * @param node Pointer to the node to allocate
 * @return COMPLETE on success, corresponding error code on failure
 */
static stat_t node_alloc(list_t list, node_t* node)
{
    void* (*alloc)(size_t) = list->alloc_fn ? list->alloc_fn : malloc;
    void (*dealloc)(void*) = list->free_fn ? list->free_fn : free;

    *node = (node_t)alloc(sizeof(node_s));
    if (!*node) return ERR_MEMORY_ALLOCATION;

    (*node)->elem = alloc(list->elem_size);
    if (!(*node)->elem) {
        dealloc(*node);
        return ERR_MEMORY_ALLOCATION;
    }

    (*node)->next = (*node)->prev = NULL;
    return COMPLETE;
}

/**
 * Initialize the list
 * 
 * @param elem_size Size of the elements in the list
 * @param free_fn Free function, NULL for default free
 * @param alloc_fn Alloc function, NULL for default malloc
 * @return Initialized list on success, NULL on failure
 */
list_t _list_init(size_t elem_size,
                  void (*free_fn)(void*),
                  void* (*alloc_fn)(size_t))
{
    list_t list = (list_t)malloc(sizeof(list_s));
    if (!list) return NULL;
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->elem_size = elem_size;

    list->free_fn = free_fn;
    list->alloc_fn = alloc_fn;

    return list;
}

/**
 * Push an element to the front of the list
 * 
 * @param list List
 * @param elem Pointer to the element to push
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_push_front(list_t list, void* elem)
{
    node_t node = NULL;
    if (node_alloc(list, &node))
        return ERR_MEMORY_ALLOCATION;

    memcpy(node->elem, elem, list->elem_size);

    if (list->size == 0) {
        list->head = node;
        list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->size++;
    return COMPLETE;
}

/**
 * Push an element to the back of the list
 * 
 * @param list List
 * @param elem Pointer to the element to push
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_push_back(list_t list, void* elem)
{
    node_t node = NULL;
    if (node_alloc(list, &node))
        return ERR_MEMORY_ALLOCATION;

    memcpy(node->elem, elem, list->elem_size);

    if (list->size == 0) {
        list->head = node;
        list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
    return COMPLETE;
}

/**
 * Unlink a node from the list
 * 
 * @param list List
 * @param node Node to unlink
 */
static void node_unlink(list_t list, node_t node)
{
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    } 

    if (node->next) {
        node->next->prev = node->prev; 
    } else {
        list->tail = node->prev;
    }

    if (list->free_fn) {
        list->free_fn(node->elem);
        list->free_fn(node);
    } else {
        free(node->elem);
        free(node);
    }
}

/**
 * Link a new node to the next node
 * 
 * @param list List
 * @param node Node to link to
 * @param new Node to link
 */
static void node_link(list_t list, node_t node, node_t link)
{
    link->prev = node->prev;
    link->next = node;
    if (node->prev) {
        link->prev->next = link;
    } else {
        list->head = link;
    }
    node->prev = link;
}

/**
 * Pop an element from the front of the list
 * 
 * @param list List
 * @param ret_elem Pointer to the element to pop, NULL to discard
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_pop_front(list_t list, void* ret_elem)
{
    if (list->size == 0) return ERR_INVALID_OPERATION; 
    
    if (ret_elem)
        memcpy(ret_elem, list->head->elem, list->elem_size);
    node_unlink(list, list->head);

    list->size--;
    return COMPLETE;
}

/**
 * Pop an element from the back of the list
 * 
 * @param list List
 * @param ret_elem Pointer to the element to pop, NULL to discard
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_pop_back(list_t list, void* ret_elem)
{
    if (list->size == 0) return ERR_INVALID_OPERATION; 
    
    if (ret_elem)
        memcpy(ret_elem, list->tail->elem, list->elem_size);
    node_unlink(list, list->tail);

    list->size--;
    return COMPLETE;
}

/**
 * Get a node from the list by index
 * 
 * @param list List
 * @param ret_node Pointer to the node
 * @param i Index of the node
 */
static void list_get_node(list_t list, node_t* ret_node, size_t i)
{
    node_t node = NULL;
    if (i < list->size / 2) {
        node = list->head;
        for (size_t j = 0; j < i; j++) {
            node = node->next;
        }
    } else {
        node = list->tail;
        for (size_t j = list->size - 1; j > i; j--) {
            node = node->prev;
        }
    }
    *ret_node = node;
}

/**
 * Insert an element into the list at a specific index
 * 
 * @param list List
 * @param elem Pointer to the element to insert
 * @param i Index to insert the element at
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_insert(list_t list, void* elem, size_t i)
{
    if (i > list->size) return ERR_INDEX_OUT_OF_RANGE;
    if (i == list->size) return list_push_back(list, elem);

    node_t node = NULL;
    list_get_node(list, &node, i);

    node_t new_node = NULL;
    if (node_alloc(list, &new_node))
        return ERR_MEMORY_ALLOCATION;

    memcpy(new_node->elem, elem, list->elem_size);
    node_link(list, node, new_node);

    list->size++;
    return COMPLETE;
}

/**
 * Remove an element from the list at a specific index
 * 
 * @param list List
 * @param ret_elem Pointer to the element to remove, NULL to discard
 * @param i Index to remove the element at
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_remove(list_t list, void* ret_elem, size_t i)
{
    if (i >= list->size) return ERR_INDEX_OUT_OF_RANGE;

    node_t node = NULL;
    list_get_node(list, &node, i);

    if (ret_elem)
        memcpy(ret_elem, node->elem, list->elem_size);
    node_unlink(list, node);

    list->size--;
    return COMPLETE;
}

/**
 * Get the first element of the list
 * 
 * @param list List
 * @param ret_elem Pointer to the element to get
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_front(list_t list, void* ret_elem)
{
    if (list->size == 0) return ERR_INVALID_OPERATION;
    memcpy(ret_elem, list->head->elem, list->elem_size);
    return COMPLETE;
}

/**
 * Get the last element of the list
 * 
 * @param list List
 * @param ret_elem Pointer to the element to get
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_back(list_t list, void* ret_elem)
{
    if (list->size == 0) return ERR_INVALID_OPERATION;
    memcpy(ret_elem, list->tail->elem, list->elem_size);
    return COMPLETE;
}

/**
 * Get an element from the list at a specific index
 * 
 * @param list List
 * @param ret_elem Pointer to the element to get
 * @param i Index of the element
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_get(list_t list, void* ret_elem, size_t i)
{
    if (i >= list->size) return ERR_INDEX_OUT_OF_RANGE;

    node_t node = NULL;
    list_get_node(list, &node, i);

    memcpy(ret_elem, node->elem, list->elem_size);
    return COMPLETE;
}

/**
 * Set an element in the list at a specific index
 * 
 * @param list List
 * @param elem Pointer to the element to set
 * @param i Index to set the element at
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_set(list_t list, void* elem, size_t i)
{
    if (i >= list->size) return ERR_INDEX_OUT_OF_RANGE;

    node_t node = NULL;
    list_get_node(list, &node, i);

    memcpy(node->elem, elem, list->elem_size);
    return COMPLETE;
}

/**
 * Concatenate two lists and clear the source
 * 
 * @param dst Destination list
 * @param src Source list
 */
void list_concat(list_t dst, list_t src)
{
    if (src->size == 0) return;
    if (dst->size == 0) {
        dst->head = src->head;
        dst->tail = src->tail;
        dst->size = src->size;
    } else {
        dst->tail->next = src->head;
        src->head->prev = dst->tail;
        dst->tail = src->tail;
        dst->size += src->size;
    }
    
    src->head = src->tail = NULL;
    src->size = 0;
}

/**
 * Copy a list
 * 
 * @param dst Pointer to the destination list
 * @param src Source list
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t list_copy(list_t* dst, list_t src)
{
    *dst = _list_init(src->elem_size, src->free_fn, src->alloc_fn);
    if (!*dst) return ERR_MEMORY_ALLOCATION;

    node_t node = src->head;
    while (node) {
        if (list_push_back(*dst, node->elem)) {
            list_deinit(*dst);
            return ERR_MEMORY_ALLOCATION;
        }
        node = node->next;
    }
    return COMPLETE;
}

/**
 * Map a function over the list
 * 
 * @param list List
 * @param fn Function to map over the list
 */
void list_map(list_t list, void (*fn)(void*))
{
    node_t node = list->head;
    while (node) {
        fn(node->elem);
        node = node->next;
    }
}

/**
 * Free the elements of the list
 * 
 * @param list List
 */
void list_clear(list_t list)
{
    node_t node = list->head;
    while (node) {
        node_t next = node->next;
        if (list->free_fn) {
            list->free_fn(node->elem);
            list->free_fn(node);
        } else {
            free(node->elem);
            free(node);
        }
        node = next;
    }
    list->head = list->tail = NULL;
    list->size = 0;
}

/**
 * Free the list and its elements
 * 
 * @param list List
 */
void list_deinit(list_t list)
{
    list_clear(list);
    free(list);
}

/**
 * Get the first iterator of the list
 * 
 * @param list List
 * @return Iterator to the first element
 */
list_it_t list_begin(list_t list)
{
    list_it_t it = (list_it_t)malloc(sizeof(list_it_s));
    if (!it) return NULL;

    it->list = list;
    it->current = list->head;
    return it;
}

/**
 * Get the last iterator of the list
 * 
 * @param list List
 * @return Iterator to the last element
 */
list_it_t list_end(list_t list)
{
    list_it_t it = (list_it_t)malloc(sizeof(list_it_s));
    if (!it) return NULL;

    it->list = list;
    it->current = list->tail;
    return it;
}

/**
 * Get the next iterator of the list
 * 
 * @param it Iterator
 * @param ret_elem Pointer to the element to get
 * @return 1 if the iterator has a next element, 0 otherwise
 */
int list_next(list_it_t it, void* ret_elem)
{
    if (!it->current) return 0;

    if (ret_elem)
        memcpy(ret_elem, it->current->elem, it->list->elem_size);
    it->current = it->current->next;
    return 1;
}

/**
 * Get the previous iterator of the list
 * 
 * @param it Iterator
 * @param ret_elem Pointer to the element to get
 * @return 1 if the iterator has a previous element, 0 otherwise
 */
int list_prev(list_it_t it, void* ret_elem)
{
    if (!it->current) return 0;

    if (ret_elem)
        memcpy(ret_elem, it->current->elem, it->list->elem_size);
    it->current = it->current->prev;
    return 1;
}

/**
 * Free the iterator
 * 
 * @param it Iterator
 */
void list_it_deinit(list_it_t it)
{
    free(it);
}
