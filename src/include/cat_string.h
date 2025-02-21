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

#ifndef __CAT_STRING_H__
#define __CAT_STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "cat_error.h"

typedef struct string_s* string_t;

size_t string_length(string_t str);
size_t string_capacity(string_t str);
char* string_to_cstr(string_t str);
char* string_at(string_t str, size_t i);
string_t string_custom(char* cstr,
                       void (*free_fn)(void*),
                       void* (*alloc_fn)(size_t));

int string_is_empty(string_t str);
int string_is_full(string_t str);
size_t string_contains(string_t str, char c);
char* string_find(string_t str, char* cstr);
char* string_kmp(string_t str, char* cstr);

stat_t string_reserve(string_t str, size_t capacity);
stat_t string_shrink_to_fit(string_t str);
stat_t string_insert(string_t str, char* cstr, size_t i);
stat_t string_remove(string_t str, size_t i, size_t len);
stat_t string_concat(string_t str1, string_t str2);
stat_t string_nconcat(string_t str1, string_t str2, size_t n);

stat_t string_copy(string_t* dst, string_t src);
stat_t string_slice(string_t str,
                    size_t start,
                    size_t end,
                    string_t* ret_string);

int string_compare(string_t str1, string_t str2);
int string_ncompare(string_t str1, string_t str2, size_t n);

void string_reverse(string_t str);
void string_to_upper(string_t str);
void string_to_lower(string_t str);
void string_to_title(string_t str);

void string_map(string_t str, void (*fn)(char*));
void string_clear(string_t str);
void string_deinit(string_t str);

#define string(cstr) \
    string_custom(cstr, NULL, NULL)

#ifdef __cplusplus
}
#endif

#endif