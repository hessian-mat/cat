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

#include "cat_string.h" 

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct string_s {
    char       *str;
    size_t      len;
    size_t      capacity;

    void      (*free_fn)(void*);
    void     *(*alloc_fn)(size_t);
} string_s;

static stat_t string_alloc(string_t str, size_t capacity);
static void gen_lps(char* cstr, size_t m, size_t* lps);

/**
 * Get the length of the string
 * 
 * @param str String
 * @return Length of the string
 */
size_t string_length(string_t str)
{
    return str->len;
}

/**
 * Get the capacity of the string
 * 
 * @param str String
 * @return Capacity of the string
 */
size_t string_capacity(string_t str)
{
    return str->capacity;
}

/**
 * Allocate memory for the string
 * 
 * @param str String
 * @param capacity Capacity of the buffer to allocate
 * @return COMPLETE on success, corresponding error code on failure
 */
static stat_t string_alloc(string_t str, size_t capacity)
{
    void* (*alloc)(size_t) = str->alloc_fn ? str->alloc_fn : malloc;
    void (*dealloc)(void*) = str->free_fn ? str->free_fn : free;
    char* s = NULL;

    if (str->str) {
        if (alloc == malloc && dealloc == free) {
            s = realloc(str->str, capacity);
            if (!s) return ERR_MEMORY_ALLOCATION;
            str->str = s;
            return COMPLETE;
        }
        s = (char*)alloc(capacity);
        if (!s) return ERR_MEMORY_ALLOCATION;

        memcpy(s, str->str, str->len + 1);
        dealloc(str->str);
    } else {
        s = (char*)alloc(capacity);
        if (!s) return ERR_MEMORY_ALLOCATION;
    }
    str->str = s;
    return COMPLETE;
}

/**
 * Initialize a string
 * 
 * @param cstr C string
 * @param free_fn Free function, NULL for default free
 * @param alloc_fn Alloc function, NULL for default malloc
 * @return Initialized string on success, NULL on failure
 */
string_t string_custom(char* cstr,
                       void (*free_fn)(void*),
                       void* (*alloc_fn)(size_t))
{
    if (strlen(cstr) >= ((size_t)0 - 2))
        return ERR_CAPACITY_OVERFLOW;
    string_t str = (string_t)malloc(sizeof(string_s));
    if (!str) return NULL;

    str->len = strlen(cstr);
    str->capacity = str->len + 1;
    str->str = NULL;

    str->alloc_fn = alloc_fn;
    str->free_fn = free_fn;

    if (string_alloc(str, str->capacity)) {
        free(str);
        return NULL;
    }
    strcpy(str->str, cstr);
    
    return str;
}

/**
 * Get the C string representation of the string
 * 
 * @param str String
 * @return C string representation of the string
 */
char* string_to_cstr(string_t str)
{
    return (char* const)str->str;
}

/**
 * Get an element from the string at a specific index
 * 
 * @param str String
 * @param i Index at which to get the element
 * @return Pointer to the element
 */
char* string_at(string_t str, size_t i)
{
    if (i >= str->len) return NULL;
    return str->str + i;
}

/**
 * Check if the string is empty
 * 
 * @param str String
 * @return 1 if the string is empty, 0 otherwise
 */
int string_is_empty(string_t str)
{
    return str->len == 0;
}

/**
 * Check if the string is full
 * 
 * @param str String
 * @return 1 if the string is full, 0 otherwise
 */
int string_is_full(string_t str)
{
    return str->len + 1 == str->capacity;
}

/**
 * Check if the string contains a character
 * 
 * @param str String
 * @param c Character to check
 * @return Number of the character in the string
 */
size_t string_contains(string_t str, char c)
{
    size_t count = 0;
    char* s = str->str;

    while (*s) {
        if (*s == c) {
            count++;
        }
        s++;
    }
    return count;
}

/**
 * Get the first occurrence of a substring in the string
 * 
 * @param str String
 * @param cstr Substring to find
 * @return Pointer to the first occurrence of the substring
 */
char* string_find(string_t str, char* cstr)
{
    if (!cstr) return str->str;
    char* s = str->str;

    while (*s) {
        char *i = s, *j = cstr;
        while (*i && *j && *i == *j) {
            i++, j++;
        }
        if (!*j) return s;
        s++;
    }
    return NULL;
}

/**
 * Generate the longest prefix suffix array
 * 
 * @param cstr Substring
 * @param m Length of the substring
 * @param lps Longest prefix suffix array
 */
static void gen_lps(char* cstr, size_t m, size_t* lps)
{
    size_t len = 0, i = 1;
    lps[0] = 0;

    while (i < m) {
        if (cstr[i] == cstr[len]) {
            lps[i++] = ++len;
        } else {
            if (len) len = lps[len - 1];
            else lps[i++] = 0;
        }
    }
}

/**
 * Get the first occurrence of a substring in the string using kmp
 * 
 * @param str String
 * @param cstr Substring to find
 * @return Pointer to the first occurrence of the substring
 */
char* string_kmp(string_t str, char* cstr)
{
    size_t n = string_length(str), m = strlen(cstr);
    if (!n) return str->str;

    size_t* lps = (size_t*)malloc(m * sizeof(size_t));
    if (!lps) return NULL;
    gen_lps(cstr, m, lps);

    size_t i = 0, j = 0;
    char* s = str->str;
    while (i < n) {
        if (s[i] == cstr[j]) {
            i++, j++;
        }
        if (j == m) {
            free(lps);
            return (s + i - j);
        }
        if (i < n && s[i] != cstr[j]) {
            if (j) j = lps[j - 1];
            else i++;
        } 
    }
    free(lps);
    return NULL;
}

/**
 * Reserve memory for the string
 * 
 * @param str String
 * @param capacity Capacity of the buffer to reserve
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t string_reserve(string_t str, size_t capacity)
{
    if (capacity <= str->capacity)
        return ERR_INVALID_OPERATION;

    if (string_alloc(str, capacity))
        return ERR_MEMORY_ALLOCATION;
    str->capacity = capacity;
    return COMPLETE;
}

/**
 * Shrink the string capacity to fit its length
 * 
 * @param str String
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t string_shrink_to_fit(string_t str)
{
    if (str->len == 0) return ERR_INVALID_OPERATION;
    if (str->capacity == str->len + 1) return COMPLETE;

    if (string_alloc(str, str->len + 1))
        return ERR_MEMORY_ALLOCATION;
    str->capacity = str->len + 1;
    return COMPLETE;
}

/**
 * Insert a substring into the string at a specific index
 * 
 * @param str String
 * @param cstr Substring to insert
 * @param i Index at which to insert the substring
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t string_insert(string_t str, char* cstr, size_t i)
{
    size_t n = strlen(cstr);
    if (i > str->len) return ERR_INDEX_OUT_OF_RANGE;
    if (n >= ((size_t)0 - 2) - str->len)
        return ERR_CAPACITY_OVERFLOW;
    if (str->len + n + 1 >= str->capacity) {
        if (string_alloc(str, str->len + n + 1))
            return ERR_MEMORY_ALLOCATION;
        str->capacity = str->len + n + 1;
    }
    if (i < str->len) {
        memmove(str->str + i + n,
                str->str + i,
                str->len - i);
    }
    memcpy(str->str + i, cstr, n);
    str->len += n;
    str->str[str->len] = '\0';

    return COMPLETE;
}

/**
 * Remove a substring from the string at a specific index
 * 
 * @param str String
 * @param i Index at which to remove the substring
 * @param len Length of the substring to remove
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t string_remove(string_t str, size_t i, size_t len)
{
    if (str->len == 0) return ERR_INVALID_OPERATION;
    if (i + len > str->len) return ERR_INDEX_OUT_OF_RANGE;

    if (i + len != str->len) {
        memmove(str->str + i,
                str->str + i + len,
                str->len - i - len);
    } 
    str->len -= len;
    str->str[str->len] = '\0';

    return COMPLETE;
}

/**
 * Copy a string with copy of the buffer
 * 
 * @param dst Pointer to the destination string
 * @param src Source string
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t string_copy(string_t* dst, string_t src)
{
    *dst = (string_t)malloc(sizeof(string_s));
    if (!*dst) return ERR_MEMORY_ALLOCATION;

    (*dst)->len = src->len;
    (*dst)->capacity = src->capacity;
    (*dst)->str = NULL;

    (*dst)->free_fn = src->free_fn;
    (*dst)->alloc_fn = src->alloc_fn;

    if (string_alloc(*dst, src->capacity)) {
        free(*dst);
        return ERR_MEMORY_ALLOCATION;
    }
    strcpy((*dst)->str, src->str);

    return COMPLETE;
}

/**
 * Compare two strings lexicographically
 * 
 * @param str1 First string
 * @param str2 Second string
 * @return 0 if strings are equal,
 *         negative value if str1 is less than str2,
 *         positive value if str1 is greater than str2
 */
int string_compare(string_t str1, string_t str2)
{
    char* s1 = str1->str;
    char* s2 = str2->str;
    while (*s1 && *s1 == *s2) {
        s1++, s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/**
 * Compare up to n characters of two strings lexicographically
 * 
 * @param str1 First string
 * @param str2 Second string
 * @param n Maximum number of characters to compare
 * @return 0 if strings are equal up to n characters,
 *         negative value if str1 is less than str2,
 *         positive value if str1 is greater than str2
 */
int string_ncompare(string_t str1, string_t str2, size_t n)
{
    char* s1 = str1->str;
    char* s2 = str2->str;
    while (n > 0 && *s1 && *s1 == *s2) {
        s1++, s2++;
        n--;
    }
    return n ? (unsigned char)*s1 - (unsigned char)*s2 : 0;
}

/**
 * Get a substring from the string
 * 
 * @param str String
 * @param start Start index of the substring
 * @param end End index of the substring
 * @param ret_string Pointer to the destination string
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t string_slice(string_t str,
                    size_t start,
                    size_t end,
                    string_t* ret_string)
{
    if (start > str->len || end > str->len) return ERR_INDEX_OUT_OF_RANGE;
    if (end <= start) return ERR_INVALID_OPERATION;

    *ret_string = (string_t)malloc(sizeof(string_s));
    if (!*ret_string) return ERR_MEMORY_ALLOCATION;

    (*ret_string)->len = end - start;
    (*ret_string)->capacity = end - start + 1;
    (*ret_string)->str = NULL;
    (*ret_string)->free_fn = str->free_fn;
    (*ret_string)->alloc_fn = str->alloc_fn;

    if (string_alloc(*ret_string, end - start + 1)) {
        free(*ret_string);
        return ERR_MEMORY_ALLOCATION;
    }
    memcpy((*ret_string)->str, str->str + start, end - start);
    (*ret_string)->str[(*ret_string)->len] = '\0';

    return COMPLETE;
}

/**
 * Concatenate two strings by appending str2 to str1
 * 
 * @param str1 Destination string to append to
 * @param str2 Source string to append from
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t string_concat(string_t str1, string_t str2)
{
    return string_insert(str1, str2->str, str1->len);
}

/**
 * Concatenate n characters from str2 to str1
 * 
 * @param str1 Destination string to append to
 * @param str2 Source string to append from
 * @param n Number of characters to concatenate from str2
 * @return COMPLETE on success, corresponding error code on failure
 */
stat_t string_nconcat(string_t str1, string_t str2, size_t n)
{
    n = (n > str2->len) ? str2->len : n;
    if (n > ((size_t)0 - 2) - str1->len)
        return ERR_CAPACITY_OVERFLOW;
    if (str1->len + n + 1 > str1->capacity) {
        if (string_alloc(str1, str1->len + n + 1))
            return ERR_MEMORY_ALLOCATION;
        str1->capacity = str1->len + n + 1;
    }
    memcpy(str1->str + str1->len, str2->str, n);
    str1->len += n;
    str1->str[str1->len] = '\0';

    return COMPLETE;
}

/**
 * Reverse a string
 * 
 * @param str String
 */
void string_reverse(string_t str)
{
    for (size_t i = 0; i < str->len / 2; i++) {
        char temp = str->str[i];
        str->str[i] = str->str[str->len - i - 1];
        str->str[str->len - i - 1] = temp;
    }
}

/**
 * Convert a string to uppercase
 * 
 * @param str String
 */
void string_to_upper(string_t str)
{
    for (size_t i = 0; i < str->len; i++) {
        str->str[i] = (char)toupper(str->str[i]);
    }
}

/**
 * Convert a string to lowercase
 * 
 * @param str String
 */
void string_to_lower(string_t str)
{
    for (size_t i = 0; i < str->len; i++) {
        str->str[i] = (char)tolower(str->str[i]);
    }
}

/**
 * Convert a string to title case
 * 
 * @param str String
 */
void string_to_title(string_t str)
{
    str->str[0] = (char)toupper(str->str[0]);
    for (size_t i = 1; i < str->len; i++) {
        str->str[i] = (char)tolower(str->str[i]);
    }
}

/**
 * Map a function over the string
 * 
 * @param str String
 * @param fn Function to map over the string
 */
void string_map(string_t str, void (*fn)(char*))
{
    for (size_t i = 0; i < str->len; i++) {
        fn(str->str + i);
    }
}

/**
 * Clear a string
 * 
 * @param str String
 */
void string_clear(string_t str)
{
    str->len = 0;
    str->str[0] = '\0';
}

/**
 * Free a string and its buffer
 * 
 * @param str String
 */
void string_deinit(string_t str)
{
    if (str->free_fn) {
        str->free_fn(str->str);
    } else {
        free(str->str);
    }
    free(str);
}
