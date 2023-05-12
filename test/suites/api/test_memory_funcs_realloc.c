#include <jansson.h>
#include <string.h>

#include "util.h"

static int malloc_called = 0;
static int realloc_called = 0;
static int free_called = 0;
static size_t malloc_used = 0;

/* helpers */
static void create_and_free_complex_object() {
    json_t *obj;

    obj = json_pack("{s:i,s:n,s:b,s:b,s:{s:s},s:[i,i,i]}", "foo", 42, "bar", "baz", 1,
                    "qux", 0, "alice", "bar", "baz", "bob", 9, 8, 7);

    json_decref(obj);
}

static void create_and_free_object_with_oom() {
    int i;
    char key[4];
    json_t *obj = json_object();

    for (i = 0; i < 10; i++) {
        snprintf(key, sizeof key, "%d", i);
        json_object_set_new(obj, key, json_integer(i));
    }

    json_decref(obj);
}

static void *my_malloc(size_t size) {
    malloc_called = 1;
    return malloc(size);
}

static void my_free(void *ptr) {
    free_called = 1;
    free(ptr);
}

static void *my_realloc(void *ptr, size_t size) {
    realloc_called = 1;
    return realloc(ptr, size);
}

static void test_simple2() {
    json_malloc_t mfunc = NULL;
    json_realloc_t rfunc = NULL;
    json_free_t ffunc = NULL;

    json_set_alloc_funcs2(my_malloc, my_realloc, my_free);
    json_get_alloc_funcs2(&mfunc, &rfunc, &ffunc);
    create_and_free_complex_object();

    if (malloc_called != 1 || free_called != 1 || mfunc != my_malloc ||
        rfunc != my_realloc || ffunc != my_free)
        fail("Custom allocation failed");
}

static void *oom_malloc(size_t size) {
    if (malloc_used + size > 800)
        return NULL;

    malloc_used += size;
    return malloc(size);
}

static void oom_free(void *ptr) {
    free_called++;
    free(ptr);
}

static void *oom_realloc(void *ptr, size_t size) {
    if (malloc_used + size > 800) {
        return NULL;
    }

    malloc_used += size;
    return realloc(ptr, size);
}

static void test_oom2() {
    free_called = 0;
    malloc_used = 0;
    json_set_alloc_funcs2(oom_malloc, oom_realloc, oom_free);
    create_and_free_object_with_oom();

    if (free_called == 0)
        fail("Allocation with OOM failed");
}

/*
  Test the secure memory functions code given in the API reference
  documentation, but by using plain memset instead of
  guaranteed_memset().
*/

static void *secure_malloc(size_t size) {
    /* Store the memory area size in the beginning of the block */
    void *ptr = malloc(size + 8);
    *((size_t *)ptr) = size;
    return (char *)ptr + 8;
}

static void *secure_realloc(void *ptr, size_t size) {
    /* Store the memory area size in the beginning of the block */
    ptr = realloc(ptr, size + 8);
    *((size_t *)ptr) = size;
    return (char *)ptr + 8;
}

static void secure_free(void *ptr) {
    size_t size;

    ptr = (char *)ptr - 8;
    size = *((size_t *)ptr);

    /*guaranteed_*/ memset(ptr, 0, size + 8);
    free(ptr);
}

static void test_secure_funcs2(void) {
    json_set_alloc_funcs2(secure_malloc, secure_realloc, secure_free);
    create_and_free_complex_object();
}

static void test_bad_args2(void) {
    /* The result of this test is not crashing. */
    json_get_alloc_funcs2(NULL, NULL, NULL);
}

static void run_tests() {
    test_simple2();
    test_secure_funcs2();
    test_oom2();
    test_bad_args2();
}
