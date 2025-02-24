#include <string.h>
#include "cat_pqueue.h"
#include "unity.h"

void setUp() {}
void tearDown() {}

int int_cmp1(const void* a, const void* b)
{
    return (*(int*)a - *(int*)b);
}

int int_cmp2(const void* a, const void* b)
{
    return (*(int*)b - *(int*)a);
}


// basic
void test1()
{
    pqueue_t pq = pqueue(int, 5, int_cmp1);
    TEST_ASSERT_NOT_NULL(pq);
    TEST_ASSERT_EQUAL_INT64(0, pqueue_size(pq));
    TEST_ASSERT_EQUAL_INT64(5, pqueue_capacity(pq));
    TEST_ASSERT_TRUE(pqueue_is_empty(pq));
    pqueue_deinit(pq);
    
    pq = pqueue(int, 5, int_cmp2);
    TEST_ASSERT_NOT_NULL(pq);
    TEST_ASSERT_EQUAL_INT64(0, pqueue_size(pq));
    TEST_ASSERT_EQUAL_INT64(5, pqueue_capacity(pq));
    TEST_ASSERT_TRUE(pqueue_is_empty(pq));
    pqueue_deinit(pq);
}

// contains
void test2()
{
    pqueue_t pq = pqueue(int, 7, int_cmp1);
    int v[] = {6, 9, 3, 4, 1, 1, 7};
    for (int i = 0; i < 7; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &v[i]));
    }
    TEST_ASSERT_TRUE(pqueue_is_full(pq));

    int v1 = 6, v2 = 9, v3 = 3, v4 = 999;
    TEST_ASSERT_EQUAL_INT64(1, pqueue_contains(pq, &v1));
    TEST_ASSERT_EQUAL_INT64(1, pqueue_contains(pq, &v2));
    TEST_ASSERT_EQUAL_INT64(1, pqueue_contains(pq, &v3));
    TEST_ASSERT_EQUAL_INT64(0, pqueue_contains(pq, &v4));

    pqueue_deinit(pq);
}

// reserve and shrink
void test3()
{
    pqueue_t pq = pqueue(int, 5, int_cmp1);

    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &i));
    }
    TEST_ASSERT_EQUAL_INT64(5, pqueue_size(pq));
    TEST_ASSERT_EQUAL_INT64(5, pqueue_capacity(pq));
    TEST_ASSERT_TRUE(pqueue_is_full(pq));

    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_reserve(pq, 10));
    TEST_ASSERT_EQUAL_INT64(5, pqueue_size(pq));
    TEST_ASSERT_EQUAL_INT64(10, pqueue_capacity(pq));
    TEST_ASSERT_FALSE(pqueue_is_full(pq));
    
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &i));
    }
    TEST_ASSERT_EQUAL_INT64(20, pqueue_capacity(pq));
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_shrink_to_fit(pq));
    TEST_ASSERT_EQUAL_INT64(15, pqueue_size(pq));
    TEST_ASSERT_EQUAL_INT64(15, pqueue_capacity(pq));
    TEST_ASSERT_TRUE(pqueue_is_full(pq));
    
    pqueue_deinit(pq);
}

// push, pop, top
void test4()
{
    pqueue_t pq = pqueue(int, 3, int_cmp1);
    int values[] = {5, 3, 7, 1, 9};
    
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &values[i]));
    }
    
    int v1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_top(pq, &v1));
    TEST_ASSERT_EQUAL_INT(1, v1);
    
    int t1[] = {1, 3, 5, 7, 9};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_pop(pq, &v1));
        TEST_ASSERT_EQUAL_INT(t1[i], v1);
    }
    
    int v2 = 99;
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &v2));
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_top(pq, &v1));
    TEST_ASSERT_EQUAL_INT(99, v1);
    TEST_ASSERT_EQUAL_INT64(1, pqueue_size(pq));
    
    pqueue_deinit(pq);
}

// min heap and max heap
void test5()
{
    pqueue_t min_pq = pqueue(int, 5, int_cmp1);
    int v[] = {5, 3, 7, 1, 9};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(min_pq, &v[i]));
    }
    
    int t1[] = {1, 3, 5, 7, 9};
    int v1;
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_pop(min_pq, &v1));
        TEST_ASSERT_EQUAL_INT(t1[i], v1);
    }
    pqueue_deinit(min_pq);
    
    pqueue_t max_pq = pqueue(int, 5, int_cmp2);
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(max_pq, &v[i]));
    }
    
    int t2[] = {9, 7, 5, 3, 1};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_pop(max_pq, &v1));
        TEST_ASSERT_EQUAL_INT(t2[i], v1);
    }
    pqueue_deinit(max_pq);
}

// merge
void test6() {
    pqueue_t pq1 = pqueue(int, 5, int_cmp1);
    pqueue_t pq2 = pqueue(int, 5, int_cmp1);
    int v1[] = {5, 3, 7, 16, 9};
    int v2[] = {10, 0, 12, 13, 14};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq1, &v1[i]));
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq2, &v2[i]));
    }
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_merge(pq1, pq2));
    TEST_ASSERT_EQUAL_INT64(10, pqueue_size(pq1));
    TEST_ASSERT_EQUAL_INT64(10, pqueue_capacity(pq1));

    int t1[] = {0, 3, 5, 7, 9, 10, 12, 13, 14, 16};
    for (int i = 0; i < 10; i++) {
        int v;
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_pop(pq1, &v));
        TEST_ASSERT_EQUAL_INT(t1[i], v);
    }
    TEST_ASSERT_TRUE(pqueue_is_empty(pq1));
    TEST_ASSERT_TRUE(pqueue_is_full(pq2));
    pqueue_deinit(pq1);
    pqueue_deinit(pq2);
}

// copy
void test7()
{
    pqueue_t pq1 = pqueue(int, 5, int_cmp1);
    int v[] = {5, 3, 7, 1, 9};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq1, &v[i]));
    }
    
    pqueue_t copy1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_copy(&copy1, pq1));
    TEST_ASSERT_EQUAL_INT64(5, pqueue_size(copy1));
    TEST_ASSERT_EQUAL_INT64(5, pqueue_capacity(copy1));
    
    for (int i = 0; i < 5; i++) {
        int v1;
        int v2;
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_pop(copy1, &v1));
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_pop(pq1, &v2));
        TEST_ASSERT_EQUAL_INT(v1, v2);
    }
    TEST_ASSERT_TRUE(pqueue_is_empty(pq1));
    TEST_ASSERT_TRUE(pqueue_is_empty(copy1));

    int v1 = 99;
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq1, &v1));
    TEST_ASSERT_FALSE(pqueue_is_empty(pq1));
    TEST_ASSERT_TRUE(pqueue_is_empty(copy1));
    
    pqueue_t pq2 = pqueue(int, 5, int_cmp2);
    pqueue_t copy2;
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_copy(&copy2, pq2));
    TEST_ASSERT_EQUAL_INT64(0, pqueue_size(copy2));
    TEST_ASSERT_EQUAL_INT64(5, pqueue_capacity(copy2));
    TEST_ASSERT_TRUE(pqueue_is_empty(copy2));

    pqueue_deinit(pq1);
    pqueue_deinit(copy1);
    pqueue_deinit(pq2);
    pqueue_deinit(copy2);
}

// stress
void test8()
{
    pqueue_t pq = pqueue(int, 10, int_cmp1);
    const int N = 100000;
    
    for (int i = N; i >= 0; i--) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &i));
    }
    TEST_ASSERT_EQUAL_INT64(N+1, pqueue_size(pq));
    
    int v, last = -1;
    while (!pqueue_is_empty(pq)) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_pop(pq, &v));
        TEST_ASSERT_TRUE(v > last);
        last = v;
    }
    TEST_ASSERT_TRUE(pqueue_is_empty(pq));
    pqueue_deinit(pq);
}

typedef struct { int x; int y; } Point;
    
int point_cmp(const void* a, const void* b)
{
    return ((Point*)a)->x - ((Point*)b)->x;
}

// struct
void test9()
{
    pqueue_t pq = pqueue(Point, 3, point_cmp);
    
    Point points[] = {{5,2}, {3,4}, {7,1}};
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &points[i]));
    }
    
    Point p;
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_top(pq, &p));
    TEST_ASSERT_EQUAL_INT(3, p.x);
    TEST_ASSERT_EQUAL_INT(4, p.y);
    
    pqueue_deinit(pq);
}

int str_cmp(const void* a, const void* b)
{
    return strcmp(*(char**)a, *(char**)b);
}

// string literals
void test10()
{
    pqueue_t pq = pqueue(char*, 5, str_cmp);
    
    char* s1 = "hello";
    char* s2 = "apple";
    char* s3 = "cat";
    char* s4 = "dog";
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &s1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &s2));
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &s3));
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_push(pq, &s4));
    
    char* v;
    TEST_ASSERT_EQUAL_INT(COMPLETE, pqueue_top(pq, &v));
    TEST_ASSERT_EQUAL_STRING("apple", v);
    
    pqueue_deinit(pq);
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