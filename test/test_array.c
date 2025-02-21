#include "cat_array.h"
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
    array_t arr = array(int, 5);
    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT64(0, array_size(arr));
    TEST_ASSERT_EQUAL_INT64(5, array_capacity(arr));
    TEST_ASSERT_TRUE(array_is_empty(arr));
    array_deinit(arr);

    arr = array(double, 10);
    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_INT64(0, array_size(arr));
    TEST_ASSERT_EQUAL_INT64(10, array_capacity(arr));
    TEST_ASSERT_TRUE(array_is_empty(arr));
    array_deinit(arr);
}

// contains
void test2()
{
    array_t arr = array(int, 4);
    int v[] = {8, 2, 3, 3, 5, 1, 3, 1};
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(arr, &v[i]));
    }
    TEST_ASSERT_TRUE(array_is_full(arr));

    int* p = array_data(arr);
    TEST_ASSERT_EQUAL_INT_ARRAY(v, p, 8);

    int v1 = 8, v2 = 2, v3 = 3, v4 = 999;
    TEST_ASSERT_EQUAL_INT64(1, array_contains(arr, &v1, NULL));
    TEST_ASSERT_EQUAL_INT64(1, array_contains(arr, &v1, int_cmp));
    TEST_ASSERT_EQUAL_INT64(1, array_contains(arr, &v2, NULL));
    TEST_ASSERT_EQUAL_INT64(1, array_contains(arr, &v2, int_cmp));
    TEST_ASSERT_EQUAL_INT64(3, array_contains(arr, &v3, NULL));
    TEST_ASSERT_EQUAL_INT64(3, array_contains(arr, &v3, int_cmp));
    TEST_ASSERT_EQUAL_INT64(0, array_contains(arr, &v4, NULL));
    TEST_ASSERT_EQUAL_INT64(0, array_contains(arr, &v4, int_cmp));

    array_deinit(arr);
}

// reserve and shrink
void test3()
{
    array_t arr = array(int, 5);
    TEST_ASSERT_EQUAL_INT(ERR_INVALID_OPERATION, array_shrink_to_fit(arr));

    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(arr, &i));
    }
    TEST_ASSERT_EQUAL_INT64(5, array_size(arr));
    TEST_ASSERT_EQUAL_INT64(5, array_capacity(arr));
    TEST_ASSERT_TRUE(array_is_full(arr));

    TEST_ASSERT_EQUAL_INT(COMPLETE, array_reserve(arr, 10));
    TEST_ASSERT_EQUAL_INT64(5, array_size(arr));
    TEST_ASSERT_EQUAL_INT64(10, array_capacity(arr));
    TEST_ASSERT_FALSE(array_is_full(arr));

    TEST_ASSERT_EQUAL_INT(COMPLETE, array_shrink_to_fit(arr));
    TEST_ASSERT_EQUAL_INT64(5, array_size(arr));
    TEST_ASSERT_EQUAL_INT64(5, array_capacity(arr));
    TEST_ASSERT_TRUE(array_is_full(arr));

    array_deinit(arr);
}

// push, pop, insert, remove
void test4()
{
    array_t arr = array(int, 3);
    int v[] = {1, 1, 4, 9};
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(arr, &v[i]));
    }

    int v1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_pop_back(arr, &v1));
    TEST_ASSERT_EQUAL_INT(9, v1);
    TEST_ASSERT_EQUAL_INT64(3, array_size(arr));
    TEST_ASSERT_EQUAL_INT64(6, array_capacity(arr));
    int t1[] = {1, 1, 4};
    int *p = array_data(arr);
    TEST_ASSERT_EQUAL_INT_ARRAY(t1, p, 3);
    
    int v2 = 5;
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_insert(arr, &v2, 1));
    TEST_ASSERT_EQUAL_INT64(4, array_size(arr));
    TEST_ASSERT_EQUAL_INT64(6, array_capacity(arr));
    int t2[] = {1, 5, 1, 4};
    p = array_data(arr);
    TEST_ASSERT_EQUAL_INT_ARRAY(t2, p, 4);
    
    int v3;
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_remove(arr, &v3, 2));
    TEST_ASSERT_EQUAL_INT(1, v3);
    TEST_ASSERT_EQUAL_INT64(3, array_size(arr));
    TEST_ASSERT_EQUAL_INT64(6, array_capacity(arr));
    int t3[] = {1, 5, 4};
    p = array_data(arr);
    TEST_ASSERT_EQUAL_INT_ARRAY(t3, p, 3);
    
    array_deinit(arr);
}

// get, set, front, back
void test5()
{
    array_t arr = array(int, 5);
    int v[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(arr, &v[i]));
    }
    
    int v1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_front(arr, &v1));
    TEST_ASSERT_EQUAL_INT(10, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_back(arr, &v1));
    TEST_ASSERT_EQUAL_INT(50, v1);

    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, array_get(arr, &v1, i));
        TEST_ASSERT_EQUAL_INT(v[i], v1);
    }
    
    int v2 = 99;
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_set(arr, &v2, 2));
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_get(arr, &v1, 2));
    TEST_ASSERT_EQUAL_INT(v2, v1);
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_set(arr, &v2, 0));
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_set(arr, &v2, 4));
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_get(arr, &v1, 0));
    TEST_ASSERT_EQUAL_INT(v2, v1);
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_get(arr, &v1, 4));
    TEST_ASSERT_EQUAL_INT(v2, v1);
    
    array_deinit(arr);
}

// copy
void test6()
{
    array_t arr1 = array(int, 10);
    int v[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(arr1, &v[i]));
    }
    
    array_t copy1;
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_copy(&copy1, arr1));
    TEST_ASSERT_EQUAL_INT64(array_size(copy1), array_size(arr1));
    TEST_ASSERT_EQUAL_INT64(array_capacity(copy1), array_capacity(arr1));
    
    int *p1 = array_data(arr1);
    int *p2 = array_data(copy1);
    TEST_ASSERT_EQUAL_INT_ARRAY(p1, p2, 5);
    
    int v1 = 99;
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_set(arr1, &v1, 2));
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_get(copy1, &v1, 2));
    TEST_ASSERT_EQUAL_INT(3, v1);
    
    array_t arr2 = array(int, 5);
    array_t copy2;
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_copy(&copy2, arr2));
    TEST_ASSERT_EQUAL_INT64(0, array_size(copy2));
    TEST_ASSERT_EQUAL_INT64(5, array_capacity(copy2));
    
    array_deinit(arr1);
    array_deinit(copy1);
    array_deinit(arr2);
    array_deinit(copy2);
}

// sort and bsearch
void test7()
{
    array_t arr = array(int, 5);
    int v[] = {5, 3, 7, 1, 9};
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(arr, &v[i]));
    }

    array_qsort(arr, int_cmp);
    int* r = array_data(arr);
    int t[] = {1, 3, 5, 7, 9};
    TEST_ASSERT_EQUAL_INT_ARRAY(t, r, 5);

    int k1 = 3;
    void* p1 = array_bsearch(arr, &k1, int_cmp);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_EQUAL_INT(3, *(int*)p1);

    int k2 = 9;
    void* p2 = array_bsearch(arr, &k2, int_cmp);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_EQUAL_INT(9, *(int*)p2);

    int k3 = 4;
    void* p3 = array_bsearch(arr, &k3, int_cmp);
    TEST_ASSERT_NULL(p3);

    array_deinit(arr);
}

// stress
void test8()
{
    array_t arr = array(int, 10);
    const int N = 100000;
    
    for (int i = 0; i < N; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(arr, &i));
    }
    TEST_ASSERT_EQUAL_INT64(N, array_size(arr));
    
    for (int i = 0; i < N; i++) {
        int v;
        TEST_ASSERT_EQUAL_INT(COMPLETE, array_get(arr, &v, i));
        TEST_ASSERT_EQUAL_INT(v, i);
    }
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_shrink_to_fit(arr));
    TEST_ASSERT_EQUAL_INT64(N, array_capacity(arr));
    
    array_deinit(arr);
}

typedef struct { int x; int y; } Point;

// struct
void test9()
{
    array_t points = array(Point, 3);
    
    Point p1 = {1, 2}, p2 = {3, 4}, p3 = {5, 6};
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(points, &p1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(points, &p2));
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(points, &p3));
    
    Point r;
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_get(points, &r, 1));
    TEST_ASSERT_EQUAL_INT(3, r.x);
    TEST_ASSERT_EQUAL_INT(4, r.y);
    
    Point new_p = {7, 8};
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_set(points, &new_p, 0));
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_get(points, &r, 0));
    TEST_ASSERT_EQUAL_INT(7, r.x);
    TEST_ASSERT_EQUAL_INT(8, r.y);
    
    array_deinit(points);
}

// string literals
void test10()
{
    array_t arr = array(char*, 10);

    char* s1 = "Hello";
    char* s2 = "World";
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(arr, &s1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_push_back(arr, &s2));

    char* v;
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_get(arr, &v, 1));
    TEST_ASSERT_EQUAL_STRING("World", v);

    char* new_str = "Cat";
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_set(arr, &new_str, 0));
    TEST_ASSERT_EQUAL_INT(COMPLETE, array_get(arr, &v, 0));
    TEST_ASSERT_EQUAL_STRING("Cat", v);

    array_deinit(arr);
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