#include <string.h>
#include "cat_hashmap.h"
#include "unity.h"

void setUp() {}
void tearDown() {}



// basic
void test1()
{
    hashmap_t ht = hashmap(int, int, 8);
    TEST_ASSERT_NOT_NULL(ht);
    TEST_ASSERT_EQUAL_INT64(8, hashmap_capacity(ht));
    TEST_ASSERT_EQUAL_INT64(0, hashmap_size(ht));
    TEST_ASSERT_TRUE(hashmap_is_empty(ht));

    for(int i = 0; i < 100; i++) {
        int k = i;
        int v = i * 10;
        TEST_ASSERT_EQUAL(COMPLETE, hashmap_assign(ht, &k, &v));
    }

    TEST_ASSERT_EQUAL_INT64(256, hashmap_capacity(ht));
    TEST_ASSERT_EQUAL_INT64(100, hashmap_size(ht));
    TEST_ASSERT_TRUE(hashmap_load(ht) < 0.75);
    TEST_ASSERT_FALSE(hashmap_is_empty(ht));

    hashmap_deinit(ht);
}

// contains
void test2()
{
    hashmap_t ht = hashmap(int, int, 8);
    int k[] = {1, 2, 3, 4, 1};
    int v[] = {10, 20, 30, 40, 50};
    
    for(int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, &k[i], &v[i]));
    }

    int k1 = 1;
    int k2 = 99;
    TEST_ASSERT_EQUAL_INT(1, hashmap_contains_key(ht, &k1));
    TEST_ASSERT_EQUAL_INT(0, hashmap_contains_key(ht, &k2));

    int v1 = 50;
    int v2 = 999;
    TEST_ASSERT_EQUAL_INT64(1, hashmap_contains_val(ht, &v1));
    TEST_ASSERT_EQUAL_INT64(0, hashmap_contains_val(ht, &v2));

    hashmap_deinit(ht);
}

// reserve
void test3()
{
    hashmap_t ht = hashmap(int, int, 4);
    TEST_ASSERT_EQUAL_INT64(8, hashmap_capacity(ht));

    for(int i = 0; i < 1000; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, &i, &i));
    }

    TEST_ASSERT_EQUAL_INT64(2048, hashmap_capacity(ht));
    TEST_ASSERT_EQUAL_INT64(1000, hashmap_size(ht));

    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_reserve(ht, 4000));
    TEST_ASSERT_EQUAL_INT64(4096, hashmap_capacity(ht));
    TEST_ASSERT_EQUAL_INT64(1000, hashmap_size(ht));

    for(int i = 0; i < 1000; i++) {
        int r;
        TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_query(ht, &i, &r));
        TEST_ASSERT_EQUAL_INT(i, r);
    }

    hashmap_deinit(ht);
}

// assign and remove
void test4()
{
    hashmap_t ht = hashmap(int, int, 8);
    int k[] = {1, 2, 3, 4, 5};
    int v[] = {10, 20, 30, 40, 50};
    
    for(int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, &k[i], &v[i]));
    }
    TEST_ASSERT_EQUAL_INT64(5, hashmap_size(ht));
    TEST_ASSERT_EQUAL_INT64(8, hashmap_capacity(ht));
    
    for(int i = 0; i < 5; i++) {
        int r;
        TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_remove(ht, &k[i], &r));
        TEST_ASSERT_EQUAL_INT(v[i], r);
        TEST_ASSERT_EQUAL_INT64(5 - (i + 1), hashmap_size(ht));
        TEST_ASSERT_FALSE(hashmap_contains_key(ht, &k[i]));
    }
    
    int k1 = 99;
    TEST_ASSERT_EQUAL_INT(ERR_INVALID_OPERATION, hashmap_remove(ht, &k1, NULL));
    
    hashmap_deinit(ht);
}

// query
void test5()
{
    hashmap_t ht = hashmap(int, char*, 8);
    int k[] = {1, 2, 3};
    char* v[] = {"one", "two", "three"};
    
    for(int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, &k[i], &v[i]));
    }
    
    for(int i = 0; i < 3; i++) {
        char* result;
        TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_query(ht, &k[i], &result));
        TEST_ASSERT_EQUAL_STRING(v[i], result);
    }
    
    int k1 = 99;
    char* r;
    TEST_ASSERT_EQUAL_INT(ERR_INVALID_OPERATION, hashmap_query(ht, &k1, &r));
    
    hashmap_deinit(ht);
}

uint64_t str_hash(const char* key)
{
    return cityhash64(key, strlen(key));
}

int str_cmp(const void* a, const void* b)
{
    return strcmp(*(const char**)a, *(const char**)b);
}

// null key
void test6()
{
    hashmap_t ht = hashmap_custom(char*, char*, 4, str_hash, str_cmp, NULL, NULL);

    char* v = "null value";
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, NULL, &v));
    TEST_ASSERT_EQUAL_INT(1, hashmap_contains_key(ht, NULL));

    char* r;
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_query(ht, NULL, &r));
    TEST_ASSERT_EQUAL_STRING(v, r);

    char* new_v = "new value";
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, NULL, &new_v));
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_query(ht, NULL, &r));
    TEST_ASSERT_EQUAL_STRING(new_v, r);

    hashmap_deinit(ht);
}

// copy
void test7()
{
    hashmap_t ht = hashmap(int, char*, 8);
    
    int k[] = {1, 2, 3};
    char* v[] = {"one", "two", "three"};
    
    for(int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, &k[i], &v[i]));
    }

    hashmap_t copy;
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_copy(&copy, ht));
    TEST_ASSERT_EQUAL_INT(hashmap_size(ht), hashmap_size(copy));

    for(int i = 0; i < 3; i++) {
        char* v1;
        char* v2;
        hashmap_query(ht, &k[i], &v1);
        hashmap_query(copy, &k[i], &v2);
        TEST_ASSERT_EQUAL_STRING(v1, v2);
    }

    int new_k = 4;
    char* new_v = "four";
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, &new_k, &new_v));
    TEST_ASSERT_EQUAL_INT(ERR_INVALID_OPERATION, hashmap_query(copy, &new_k, &new_v));

    hashmap_deinit(ht);
    hashmap_deinit(copy);
}

// stress
void test8()
{
    hashmap_t ht = hashmap(int, int, 8);
    const int N = 100000;
    
    for(int i = 0; i < N; i++) {
        int v = i * 10;
        TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, &i, &v));
    }
    TEST_ASSERT_EQUAL_INT64(N, hashmap_size(ht));
    TEST_ASSERT_TRUE(hashmap_capacity(ht) >= N / 0.75);
    
    for(int i = 0; i < N; i++) {
        int r;
        TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_remove(ht, &i, &r));
        TEST_ASSERT_EQUAL_INT(i * 10, r);
    }
    TEST_ASSERT_TRUE(hashmap_is_empty(ht));
    
    hashmap_deinit(ht);
}

typedef struct { int x; int y; } Point;

int point_cmp(const void* a, const void* b) {
    const Point* p1 = (const Point*)a;
    const Point* p2 = (const Point*)b;
    return (p1->x != p2->x || p1->y != p2->y);
}

// struct
void test9()
{
    hashmap_t ht = hashmap_custom(Point, char*, 4, NULL, point_cmp, NULL, NULL);
    
    Point p1 = {1, 2};
    Point p2 = {3, 4};
    char* v1 = "Point 1";
    char* v2 = "Point 2";
    
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, &p1, &v1));
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_assign(ht, &p2, &v2));
    
    char* r;
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_query(ht, &p1, &r));
    TEST_ASSERT_EQUAL_STRING(v1, r);
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_query(ht, &p2, &r));
    TEST_ASSERT_EQUAL_STRING(v2, r);
    
    Point p3 = {1, 2};
    char* v;
    TEST_ASSERT_EQUAL_INT(1, hashmap_contains_key(ht, &p3));
    TEST_ASSERT_EQUAL_INT(COMPLETE, hashmap_query(ht, &p3, &v));
    TEST_ASSERT_EQUAL_STRING(v1, v);
    
    hashmap_deinit(ht);
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