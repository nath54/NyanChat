#include "unity.h"
#include "hashmap.h"

void setUp(void) {}
void tearDown(void) {}

void test_hashmap_create(void)
{
    Hashmap *h = hashmap_create(4);
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_NOT_NULL(h->items);
    hashmap_free(h);
}

void test_hashmap_insert(void)
{
    Hashmap *h = hashmap_create(4);
    TEST_ASSERT_EQUAL_STRING(NULL, h->items[2].key);
    TEST_ASSERT_EQUAL(0, h->items[2].value);
    hashmap_insert(h, "a", 2);
    TEST_ASSERT_EQUAL_STRING("a", h->items[2].key);
    TEST_ASSERT_EQUAL(2, h->items[2].value);
    hashmap_insert(h, "a", 2);
    hashmap_insert(h, "a", 2);
    hashmap_insert(h, "a", 2);
    TEST_ASSERT_EQUAL(4, h->size);
    TEST_ASSERT_EQUAL(1, h->count);
    hashmap_free(h);
}

void test_hashmap_find(void)
{
    Hashmap *h = hashmap_create(4);
    TEST_ASSERT_EQUAL(-1, hashmap_find(h, "a"));
    hashmap_insert(h, "a", 2);
    hashmap_insert(h, "b", 10);
    TEST_ASSERT_EQUAL(2, hashmap_find(h, "a"));
    hashmap_free(h);
}

void test_hashmap_get(void)
{
    Hashmap *h = hashmap_create(4);
    TEST_ASSERT_EQUAL(0, hashmap_get(h, "a"));
    hashmap_insert(h, "a", 2);
    TEST_ASSERT_EQUAL(2, hashmap_get(h, "a"));
    hashmap_free(h);
}

void test_hashmap_increase(void)
{
    Hashmap *h = hashmap_create(4);
    hashmap_insert(h, "a", 2);
    hashmap_increase(h, "a", 4);
    uint i_a = hashmap_find(h, "a");
    TEST_ASSERT_EQUAL(6, h->items[i_a].value);
    hashmap_increase(h, "b", 6);
    uint i_b = hashmap_find(h, "b");
    TEST_ASSERT_EQUAL(6, h->items[i_b].value);
    hashmap_free(h);
}

void test_hashmap_resize(void)
{
    Hashmap *h = hashmap_create(4);
    hashmap_insert(h, "a", 2);
    hashmap_insert(h, "b", 10);
    hashmap_insert(h, "c", 8);
    hashmap_insert(h, "d", 14);
    TEST_ASSERT_EQUAL(8, h->size);
    TEST_ASSERT_EQUAL(10, hashmap_get(h, "b"));
    hashmap_free(h);
}

void test_hashmap_remove(void)
{
    Hashmap *h = hashmap_create(4);
    hashmap_insert(h, "a", 2);
    hashmap_insert(h, "b", 10);
    hashmap_insert(h, "c", 8);
    hashmap_insert(h, "d", 14);
    uint index = hashmap_find(h, "b");
    uint value = hashmap_remove(h, "b");
    TEST_ASSERT_EQUAL(10, value);
    TEST_ASSERT_EQUAL_STRING(TOMBSTONE, h->items[index].key);
    hashmap_free(h);
}


int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_hashmap_create);
    RUN_TEST(test_hashmap_insert);
    RUN_TEST(test_hashmap_find);
    RUN_TEST(test_hashmap_get);
    RUN_TEST(test_hashmap_increase);
    RUN_TEST(test_hashmap_resize);
    RUN_TEST(test_hashmap_remove);
    return UNITY_END();
}