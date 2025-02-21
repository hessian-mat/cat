#include "cat_list.h"
#include "unity.h"

void setUp() {}
void tearDown() {}

int int_cmp(const void* a, const void* b)
{
    return (*(int*)a - *(int*)b);
}

// basic
void test1()
{
    list_t list = list(int);
    TEST_ASSERT_NOT_NULL(list);
    TEST_ASSERT_EQUAL_INT64(0, list_size(list));
    TEST_ASSERT_TRUE(list_is_empty(list));
    list_deinit(list);

    list = list(short);
    TEST_ASSERT_NOT_NULL(list);
    TEST_ASSERT_EQUAL_INT64(0, list_size(list));
    TEST_ASSERT_TRUE(list_is_empty(list));
    list_deinit(list);
}

// contains
void test2()
{
    list_t list = list(int);
    int v[] = {1, 2, 3, 1, 5};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &v[i]));
    }

    int v1 = 1, v2 = 2, v3 = 999;
    TEST_ASSERT_EQUAL_INT64(2, list_contains(list, &v1, NULL));
    TEST_ASSERT_EQUAL_INT64(2, list_contains(list, &v1, int_cmp));
    TEST_ASSERT_EQUAL_INT64(1, list_contains(list, &v2, NULL));
    TEST_ASSERT_EQUAL_INT64(1, list_contains(list, &v2, int_cmp));
    TEST_ASSERT_EQUAL_INT64(0, list_contains(list, &v3, NULL));
    TEST_ASSERT_EQUAL_INT64(0, list_contains(list, &v3, int_cmp));
    list_deinit(list);
}

// push and pop
void test3()
{
    list_t list = list(int);
    int v[] = {10, 20, 30};
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_front(list, &v[0]));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &v[2]));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_insert(list, &v[1], 1));
    TEST_ASSERT_EQUAL_INT64(3, list_size(list));
    
    int v1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_pop_front(list, &v1));
    TEST_ASSERT_EQUAL_INT(10, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_pop_back(list, &v1));
    TEST_ASSERT_EQUAL_INT(30, v1);
    TEST_ASSERT_EQUAL_INT64(1, list_size(list));
    
    list_clear(list);
    TEST_ASSERT_TRUE(list_is_empty(list));
    TEST_ASSERT_EQUAL_INT(ERR_INVALID_OPERATION, list_pop_front(list, NULL));
    TEST_ASSERT_EQUAL_INT(ERR_INVALID_OPERATION, list_pop_back(list, NULL));
    
    list_deinit(list);
}

// insert, remove
void test4()
{
    list_t list = list(int);
    int v[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &v[i]));
    }
    
    int v1 = 99;
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_insert(list, &v1, 2));
    int v2;
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_remove(list, &v2, 2));
    TEST_ASSERT_EQUAL_INT(99, v2);
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_insert(list, &v1, 0));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_insert(list, &v1, 6));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_remove(list, NULL, 0));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_remove(list, NULL, 5));

    int t[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        int v3;
        TEST_ASSERT_EQUAL_INT(COMPLETE, list_pop_front(list, &v3));
        TEST_ASSERT_EQUAL_INT(t[i], v3);
    }
    TEST_ASSERT_TRUE(list_is_empty(list));
    
    list_deinit(list);
}

// get, set, front, back
void test5()
{
    list_t list = list(int);
    int v[] = {10, 20, 30};
    
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &v[i]));
    }
    
    int v1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_get(list, &v1, 1));
    TEST_ASSERT_EQUAL_INT(20, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_front(list, &v1));
    TEST_ASSERT_EQUAL_INT(10, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_back(list, &v1));
    TEST_ASSERT_EQUAL_INT(30, v1);
    
    int v2 = 99;
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_set(list, &v2, 1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_get(list, &v1, 1));
    TEST_ASSERT_EQUAL_INT(99, v1);
    
    TEST_ASSERT_EQUAL_INT(ERR_INDEX_OUT_OF_RANGE, list_get(list, &v1, 3));
    TEST_ASSERT_EQUAL_INT(ERR_INDEX_OUT_OF_RANGE, list_set(list, &v2, 3));
    
    list_deinit(list);
}

// copy
void test6()
{
    list_t list1 = list(int);
    int v[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list1, &v[i]));
    }
    
    list_t copy1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_copy(&copy1, list1));
    TEST_ASSERT_EQUAL_INT64(3, list_size(copy1));
    
    int v1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_get(copy1, &v1, 0));
    TEST_ASSERT_EQUAL_INT(1, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_set(list1, &v1, 0));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_get(copy1, &v1, 0));
    TEST_ASSERT_EQUAL_INT(1, v1);
    
    list_t list2 = list(int);
    list_t copy2;
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_copy(&copy2, list2));
    TEST_ASSERT_TRUE(list_is_empty(copy2));
    
    list_deinit(list1);
    list_deinit(copy1);
    list_deinit(list2);
    list_deinit(copy2);
}

// stress
void test7()
{
    list_t list = list(int);
    const int N = 100000;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < N; j++) {
            TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &j));
        }
        TEST_ASSERT_EQUAL_INT64(N, list_size(list));
        
        for (int j = 0; j < N; j++) {
            int v;
            TEST_ASSERT_EQUAL_INT(COMPLETE, list_pop_front(list, &v));
            TEST_ASSERT_EQUAL_INT(j, v);
        }
        TEST_ASSERT_TRUE(list_is_empty(list));
    }
    
    list_deinit(list);
}

typedef struct {
    int id;
    float score;
    char name[20];
} Stu;

// struct
void test8()
{
    list_t list = list(Stu);
    
    Stu s1 = {1, 90.5f, "bob"};
    Stu s2 = {2, 85.0f, "alice"};
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &s1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &s2));
    
    Stu v;
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_get(list, &v, 0));
    TEST_ASSERT_EQUAL_INT(1, v.id);
    TEST_ASSERT_EQUAL_FLOAT(90.5f, v.score);
    TEST_ASSERT_EQUAL_STRING("bob", v.name);
    
    Stu v2 = {3, 95.0f, "mike"};
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_set(list, &v2, 1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_get(list, &v, 1));
    TEST_ASSERT_EQUAL_STRING("mike", v.name);
    
    list_deinit(list);
}

void test9()
{
    list_t list = list(char*);
    
    char* s1 = "hello";
    char* s2 = "world";
    char* s3 = "test";
    char* s4 = "i like cats";
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &s1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &s2));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &s3));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_push_back(list, &s4));
    
    char* v;
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_get(list, &v, 0));
    TEST_ASSERT_EQUAL_STRING("hello", v);
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_get(list, &v, 3));
    TEST_ASSERT_EQUAL_STRING("i like cats", v);
    
    char* new_s = "new";
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_set(list, &new_s, 2));
    TEST_ASSERT_EQUAL_INT(COMPLETE, list_get(list, &v, 2));
    TEST_ASSERT_EQUAL_STRING("new", v);
    
    list_deinit(list);
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
    return UNITY_END();
}
