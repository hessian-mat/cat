#include "cat_string.h"
#include "unity.h"

void setUp() {}
void tearDown() {}

void test1()
{
    string_t str = string("Hello");
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_INT64(5, string_length(str));
    TEST_ASSERT_EQUAL_INT64(6, string_capacity(str));
    TEST_ASSERT_FALSE(string_is_empty(str));
    
    string_clear(str);
    TEST_ASSERT_TRUE(string_is_empty(str));
    TEST_ASSERT_EQUAL_INT64(0, string_length(str));
    
    string_deinit(str);
}

void test2()
{
    string_t str = string("HelloWorld");
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_insert(str, ", ", 5));
    TEST_ASSERT_EQUAL_STRING("Hello, World", string_to_cstr(str));
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_remove(str, 5, 2));
    TEST_ASSERT_EQUAL_STRING("HelloWorld", string_to_cstr(str));
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_insert(str, "Hey ", 0));
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_insert(str, "!", string_length(str)));
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_remove(str, 0, 4));
    TEST_ASSERT_EQUAL_STRING("HelloWorld!", string_to_cstr(str));

    string_deinit(str);
}

void test3()
{
    string_t str = string("Hello, World!");
    
    TEST_ASSERT_EQUAL_INT64(3, string_contains(str, 'l'));
    TEST_ASSERT_NOT_NULL(string_find(str, "World"));
    TEST_ASSERT_EQUAL_STRING(string_to_cstr(str), string_kmp(str, "Hello"));
    
    string_deinit(str);
}

void test4()
{
    string_t str = string("hello WORLD");
    string_to_upper(str);
    TEST_ASSERT_EQUAL_STRING("HELLO WORLD", string_to_cstr(str));
    
    string_to_lower(str);
    TEST_ASSERT_EQUAL_STRING("hello world", string_to_cstr(str));
    
    string_to_title(str);
    TEST_ASSERT_EQUAL_STRING("Hello world", string_to_cstr(str));
    
    string_deinit(str);
}

void test5()
{
    string_t src = string("Hello");
    string_t copy;
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_copy(&copy, src));
    TEST_ASSERT_EQUAL_STRING(string_to_cstr(src), string_to_cstr(copy));
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_insert(src, "World", 0));
    TEST_ASSERT_FALSE(string_compare(src, copy) == 0);
    
    string_deinit(src);
    string_deinit(copy);
}

void test6()
{
    string_t s1 = string("Hello");
    string_t s2 = string("World");
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_concat(s1, s2));
    TEST_ASSERT_EQUAL_STRING("HelloWorld", string_to_cstr(s1));
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_nconcat(s1, s2, 3));
    TEST_ASSERT_EQUAL_STRING("HelloWorldWor", string_to_cstr(s1));
    
    string_deinit(s1);
    string_deinit(s2);
}

void test7()
{
    string_t str = string("Test");
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_reserve(str, 20));
    TEST_ASSERT_EQUAL_INT64(20, string_capacity(str));
    TEST_ASSERT_EQUAL_INT64(4, string_length(str));
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_shrink_to_fit(str));
    TEST_ASSERT_EQUAL_INT64(5, string_capacity(str));
    TEST_ASSERT_EQUAL_INT64(4, string_length(str));
    TEST_ASSERT_TRUE(string_is_full(str));
    
    string_deinit(str);
}

void test8()
{
    string_t str = string("HelloWorld");
    string_t slice;
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_slice(str, 5, 10, &slice));
    TEST_ASSERT_EQUAL_STRING("World", string_to_cstr(slice));
    
    TEST_ASSERT_EQUAL_INT(ERR_INDEX_OUT_OF_RANGE, string_slice(str, 10, 15, &slice));
    TEST_ASSERT_EQUAL_INT(ERR_INVALID_OPERATION, string_slice(str, 3, 2, &slice));
    
    string_deinit(str);
    string_deinit(slice);
}

void test9()
{
    string_t s1 = string("apple");
    string_t s2 = string("appli");
    
    TEST_ASSERT_TRUE(string_compare(s1, s2) < 0);
    TEST_ASSERT_EQUAL_INT(0, string_ncompare(s1, s2, 3));
    
    string_deinit(s1);
    string_deinit(s2);
}

void test10()
{
    string_t str = string("Hello");
    string_reverse(str);
    TEST_ASSERT_EQUAL_STRING("olleH", string_to_cstr(str));
    
    string_clear(str);
    string_reverse(str);
    TEST_ASSERT_TRUE(string_is_empty(str));
    
    string_deinit(str);
}

void test11()
{
    string_t empty = string("");
    TEST_ASSERT_TRUE(string_is_empty(empty));
    
    TEST_ASSERT_EQUAL_INT(ERR_INVALID_OPERATION, string_remove(empty, 0, 1));
    
    string_t full = string("1234");
    TEST_ASSERT_TRUE(string_is_full(full));
    TEST_ASSERT_EQUAL_INT(COMPLETE, string_insert(full, "5", 4));
    TEST_ASSERT_TRUE(string_is_full(full));
    
    string_deinit(empty);
    string_deinit(full);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test1);
    RUN_TEST(test2);
    RUN_TEST(test3);
    RUN_TEST(test4);
    RUN_TEST(test5);
    RUN_TEST(test6);
    RUN_TEST(test7);
    RUN_TEST(test8);
    RUN_TEST(test9);
    RUN_TEST(test10);
    RUN_TEST(test11);
    return UNITY_END();
} 