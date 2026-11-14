// os_log_create: one immortal object per subsystem and category. Run by run-create.sh against the real os_log_create.c.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include "os_log_s.h"

#define GLOBAL_REFCNT INT_MAX /* libdispatch's _OS_OBJECT_GLOBAL_REFCNT: retain and release skip such an object */

const struct { int unused; } OS_os_log_class;
static volatile int allocations;

void *_os_object_alloc(const void *cls, size_t size)
{
	__atomic_fetch_add(&allocations, 1, __ATOMIC_SEQ_CST);
	return calloc(1, size);
}

volatile int spares_released;

void os_release(void *object)
{
	__atomic_fetch_add(&spares_released, 1, __ATOMIC_SEQ_CST);
	// what the os_log class's -_dispose does before the object is freed
	free((void *)((os_log_t)object)->subsystem);
	free((void *)((os_log_t)object)->category);
	free(object);
}

void _libtrace_assert_fail(const char *message, ...)
{
	fprintf(stderr, "%s\n", message);
	abort();
}

#define THREADS 8
static os_log_t raced[THREADS];
static volatile int go;

static void *racer(void *arg)
{
	long i = (long)arg;
	while (!go) {}
	raced[i] = os_log_create("com.example.race", "same");
	return NULL;
}

int main(void)
{
	os_log_t a = os_log_create("com.example.sub", "one");
	os_log_t b = os_log_create("com.example.sub", "one");
	os_log_t other_category = os_log_create("com.example.sub", "two");
	os_log_t other_subsystem = os_log_create("com.example.other", "one");
	os_log_t swapped = os_log_create("one", "com.example.sub");

	assert(a == b);
	assert(a != other_category && a != other_subsystem && a != swapped);
	assert(other_category != other_subsystem && other_category != swapped && other_subsystem != swapped);
	assert(allocations == 4);

	assert(a->ref_cnt == GLOBAL_REFCNT && a->xref_cnt == GLOBAL_REFCNT);
	assert(other_category->ref_cnt == GLOBAL_REFCNT && other_category->xref_cnt == GLOBAL_REFCNT);
	assert(a->magic == OS_LOG_MAGIC);
	assert(strcmp(a->subsystem, "com.example.sub") == 0 && strcmp(a->category, "one") == 0);

	for (int i = 0; i < 1000; i++)
		assert(os_log_create("com.example.sub", "one") == a);
	assert(allocations == 4);

	// Threads that all miss the lookup each allocate; one wins and the rest release their spare.
	int before = allocations, released_before = spares_released;
	pthread_t threads[THREADS];
	for (long i = 0; i < THREADS; i++)
		pthread_create(&threads[i], NULL, racer, (void *)i);
	go = 1;
	for (int i = 0; i < THREADS; i++)
		pthread_join(threads[i], NULL);
	for (int i = 1; i < THREADS; i++)
		assert(raced[i] == raced[0]);
	assert(raced[0]->xref_cnt == GLOBAL_REFCNT);
	assert((allocations - before) - (spares_released - released_before) == 1);
	puts("os-log-create: ok");
	return 0;
}
