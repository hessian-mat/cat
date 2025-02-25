#include "cat_deque.h"
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
    deque_t deq = deque(int, 5);
    TEST_ASSERT_NOT_NULL(deq);
    TEST_ASSERT_EQUAL_INT64(0, deque_size(deq));
    TEST_ASSERT_EQUAL_INT64(5, deque_capacity(deq));
    TEST_ASSERT_TRUE(deque_is_empty(deq));
    deque_deinit(deq);
    
    deq = deque(char, 10);
    TEST_ASSERT_NOT_NULL(deq);
    TEST_ASSERT_EQUAL_INT64(0, deque_size(deq));
    TEST_ASSERT_EQUAL_INT64(10, deque_capacity(deq));
    TEST_ASSERT_TRUE(deque_is_empty(deq));
    deque_deinit(deq);
}

// contains
void test2()
{
    deque_t deq = deque(int, 3);
    int v[] = {3, 1, 2, 3, 3, 4};
    for (int i = 0; i < 6; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq, &v[i]));
    }
    TEST_ASSERT_TRUE(deque_is_full(deq));

    int v1 = 3, v2 = 1, v3 = 2, v4 = 999;
    TEST_ASSERT_EQUAL_INT64(3, deque_contains(deq, &v1, NULL));
    TEST_ASSERT_EQUAL_INT64(3, deque_contains(deq, &v1, int_cmp));
    TEST_ASSERT_EQUAL_INT64(1, deque_contains(deq, &v2, NULL));
    TEST_ASSERT_EQUAL_INT64(1, deque_contains(deq, &v2, int_cmp));
    TEST_ASSERT_EQUAL_INT64(1, deque_contains(deq, &v3, NULL));
    TEST_ASSERT_EQUAL_INT64(1, deque_contains(deq, &v3, int_cmp));
    TEST_ASSERT_EQUAL_INT64(0, deque_contains(deq, &v4, NULL));
    TEST_ASSERT_EQUAL_INT64(0, deque_contains(deq, &v4, int_cmp));
    
    deque_deinit(deq);
}

// reserve and shrink
void test3()
{
    deque_t deq = deque(int, 5);
    TEST_ASSERT_EQUAL_INT(ERR_INVALID_OPERATION, deque_shrink_to_fit(deq));

    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq, &i));
    }
    TEST_ASSERT_EQUAL_INT64(5, deque_size(deq));
    TEST_ASSERT_EQUAL_INT64(5, deque_capacity(deq));
    TEST_ASSERT_TRUE(deque_is_full(deq));

    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_reserve(deq, 20));
    TEST_ASSERT_EQUAL_INT64(5, deque_size(deq));
    TEST_ASSERT_EQUAL_INT64(20, deque_capacity(deq));
    TEST_ASSERT_FALSE(deque_is_full(deq));

    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_shrink_to_fit(deq));
    TEST_ASSERT_EQUAL_INT64(5, deque_size(deq));
    TEST_ASSERT_EQUAL_INT64(5, deque_capacity(deq));
    TEST_ASSERT_TRUE(deque_is_full(deq));

    deque_deinit(deq);
}

// push and pop
void test4()
{
    deque_t deq = deque(int, 5);
    int v[] = {0, 8, 6, 4, 7, 4};
    for (int i = 0; i < 6; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq, &v[i]));
    }

    int v1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_pop_back(deq, &v1));
    TEST_ASSERT_EQUAL_INT(4, v1);
    TEST_ASSERT_EQUAL_INT64(5, deque_size(deq));
    TEST_ASSERT_EQUAL_INT64(10, deque_capacity(deq));
    int t[] = {0, 8, 6, 4, 7};
    
    for (int i = 0; i < 5; i++) {
        int v2;
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_pop_front(deq, &v2));
        TEST_ASSERT_EQUAL_INT(t[i], v2);
    }
    TEST_ASSERT_TRUE(deque_is_empty(deq));

    for (int i = 0; i < 6; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_front(deq, &v[i]));
    }
    TEST_ASSERT_EQUAL_INT64(6, deque_size(deq));
    TEST_ASSERT_EQUAL_INT64(10, deque_capacity(deq));
    TEST_ASSERT_FALSE(deque_is_empty(deq));

    for (int i = 0; i < 6; i++) {
        int v3;
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_pop_back(deq, &v3));
        TEST_ASSERT_EQUAL_INT(v[i], v3);
    }
    TEST_ASSERT_TRUE(deque_is_empty(deq));

    deque_deinit(deq);
}

// get, set, front, back
void test5()
{
    deque_t deq = deque(int, 5);
    int v[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq, &v[i]));
    }
    
    int v1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_front(deq, &v1));
    TEST_ASSERT_EQUAL_INT(10, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_back(deq, &v1));
    TEST_ASSERT_EQUAL_INT(50, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_get(deq, &v1, 0));
    TEST_ASSERT_EQUAL_INT(10, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_get(deq, &v1, deque_size(deq) - 1));
    TEST_ASSERT_EQUAL_INT(50, v1);

    int v2 = 99;
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_set(deq, &v2, 0));
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_get(deq, &v1, 0));
    TEST_ASSERT_EQUAL_INT(99, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_set(deq, &v2, deque_size(deq) - 1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_get(deq, &v1, deque_size(deq) - 1));
    TEST_ASSERT_EQUAL_INT(99, v1);

    deque_deinit(deq);
}

// concat
void test6()
{
    deque_t deq1 = deque(int, 5);
    deque_t deq2 = deque(int, 3);
    int v[] = {0, 1, 2, 3, 4, 5, 6, 7};
    for (int i = 2; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq1, &v[i]));
    }
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_front(deq1, &v[1]));
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_front(deq1, &v[0]));
    for (int i = 5; i < 8; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq2, &i));
    }
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_concat(deq1, deq2));
    TEST_ASSERT_EQUAL_INT64(8, deque_size(deq1));

    int t[] = {0, 1, 2, 3, 4, 5, 6, 7};
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_pop_front(deq1, &t[i]));
    }
    TEST_ASSERT_FALSE(deque_is_empty(deq2));
    TEST_ASSERT_EQUAL_INT64(3, deque_size(deq2));
    TEST_ASSERT_EQUAL_INT64(8, deque_capacity(deq1));

    deque_deinit(deq1);
    deque_deinit(deq2);
}

// copy
void test7()
{
    deque_t deq1 = deque(int, 4);
    int v[] = {1, 3, 2};
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq1, &v[i]));
    }
    
    deque_t copy1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_copy(&copy1, deq1));
    TEST_ASSERT_EQUAL_INT64(deque_size(deq1), deque_size(copy1));
    TEST_ASSERT_EQUAL_INT64(deque_capacity(deq1), deque_capacity(copy1));
    
    int v1 = 9;
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_set(deq1, &v1, 0));
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_get(deq1, &v1, 0));
    TEST_ASSERT_EQUAL_INT(9, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_get(copy1, &v1, 0));
    TEST_ASSERT_EQUAL_INT(1, v1);

    deque_t deq2 = deque(int, 3);
    deque_t copy2;
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_copy(&copy2, deq2));
    TEST_ASSERT_EQUAL_INT64(0, deque_size(copy2));
    TEST_ASSERT_EQUAL_INT64(3, deque_capacity(copy2));

    deque_deinit(deq1);
    deque_deinit(copy1);
    deque_deinit(deq2);
    deque_deinit(copy2);
}

// stress
void test8()
{
    deque_t deq = deque(int, 10);
    const int N = 100000;
    
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq, &i));
        else TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_front(deq, &i));
    }
    TEST_ASSERT_EQUAL_INT64(N, deque_size(deq));
    
    for (int i = 0; i < N; i++) {
        int v;
        if (i % 2 == 0) TEST_ASSERT_EQUAL_INT(COMPLETE, deque_pop_back(deq, &v));
        else TEST_ASSERT_EQUAL_INT(COMPLETE, deque_pop_front(deq, &v));
    }
    TEST_ASSERT_TRUE(deque_is_empty(deq));
    
    deque_deinit(deq);
}

typedef struct { int x; int y; } Point;

// struct
void test9()
{
    deque_t deq = deque(Point, 3);
    
    Point p = {1, 2};
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq, &p));
    p.x = 3;
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_front(deq, &p));
    
    Point r;
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_front(deq, &r));
    TEST_ASSERT_EQUAL_INT(3, r.x);
    TEST_ASSERT_EQUAL_INT(2, r.y);
    
    deque_deinit(deq);
}

// string literals
void test10()
{
    deque_t deq = deque(char*, 4);
    
    char* s1 = "hello";
    char* s2 = "world";
    char* s3 = "cat";
    char* s4 = "dog";
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq, &s1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_front(deq, &s2));
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_back(deq, &s3));
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_push_front(deq, &s4));
    
    char* v;
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_front(deq, &v));
    TEST_ASSERT_EQUAL_STRING("dog", v);
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_get(deq, &v, 2));
    TEST_ASSERT_EQUAL_STRING("hello", v);
    
    char* new_s = "test";
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_set(deq, &new_s, 1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, deque_get(deq, &v, 1));
    TEST_ASSERT_EQUAL_STRING("test", v);
    
    deque_deinit(deq);
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
    return UNITY_END();
} 