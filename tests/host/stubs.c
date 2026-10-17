#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// Stand-ins for the Objective-C runtime and the assert hook, so unpack_buffer.m runs on a plain host.
// A receiver is a struct stub; its kind decides what a message to it returns.
struct stub { int kind; };
enum { PLAIN = 1, DESCRIBED = 2, UNCONVERTIBLE = 3 };

struct stub unconvertible = { UNCONVERTIBLE };

void _libtrace_assert_fail(const char *message, ...)
{
	va_list ap;
	va_start(ap, message);
	vfprintf(stderr, message, ap);
	va_end(ap);
	fputc('\n', stderr);
	abort();
}

const char *object_getClassName(void *object) { return "Stub"; }

// -description answers nil for PLAIN and an object whose UTF8String is NULL for DESCRIBED;
// -UTF8String of that object answers NULL. Both messages arrive here, told apart by receiver kind.
void *objc_msgSend(struct stub *receiver, void *sel)
{
	return receiver->kind == DESCRIBED ? (void *)&unconvertible : NULL;
}

void *objc_autoreleasePoolPush(void) { return 0; }
void objc_autoreleasePoolPop(void *pool) {}
void __objc_exec_class(void *module) {}
