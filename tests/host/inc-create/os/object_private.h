#pragma once
#include <limits.h>
#include <stddef.h>
// Mirrors libdispatch's os/object_private.h: the header every object starts with, and the count that marks an
// object retain and release leave alone.
#define _OS_OBJECT_GLOBAL_REFCNT INT_MAX
#define _OS_OBJECT_HEADER(isa, ref_cnt, xref_cnt) \
	isa; \
	int volatile ref_cnt; \
	int volatile xref_cnt
#define OS_OBJECT_CLASS_SYMBOL(name) OS_##name##_class
#define OS_OBJECT_OBJC_CLASS_DECL(name) extern const struct { int unused; } OS_OBJECT_CLASS_SYMBOL(name)
void *_os_object_alloc(const void *cls, size_t size);
