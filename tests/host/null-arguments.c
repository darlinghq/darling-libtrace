#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// os_log_decode_buffer must never dereference a NULL pointer argument or strdup(NULL).
// Item header byte = (type << 4) | privacy; types: 1 count, 2 string, 3 pointer, 4 object.

char *os_log_decode_buffer(const char *formatString, uint8_t *buffer, uint32_t bufferSize);

struct stub { int kind; };
extern struct stub unconvertible;

static int failures = 0;
static uint8_t buf[128];
static int n;

static void begin(int count) { n = 0; buf[n++] = 0; buf[n++] = (uint8_t)count; }
static void putItem(uint8_t type, const void *data, uint8_t size)
{
	buf[n++] = (uint8_t)(type << 4 | 2);
	buf[n++] = size;
	memcpy(&buf[n], data, size);
	n += size;
}
static void putPointer(uint8_t type, const void *pointer)
{
	uint64_t p = (uint64_t)(uintptr_t)pointer;
	putItem(type, &p, sizeof p);
}
static void putCount(uint32_t count) { putItem(1, &count, sizeof count); }

static void check(const char *name, const char *format, const char *want, int prefix)
{
	char *got = os_log_decode_buffer(format, buf, (uint32_t)n);
	int ok = prefix ? strncmp(got, want, strlen(want)) == 0 : strcmp(got, want) == 0;
	if (!ok) {
		fprintf(stderr, "FAIL %s: got [%s], want [%s]\n", name, got, want);
		failures++;
	}
	free(got);
}

int main(void)
{
	static const uint8_t bytes[] = { 1, 2, 3 };
	static const wchar_t wide[] = L"hi";
	struct stub plain = { 1 }, described = { 2 };

	begin(1); putPointer(2, NULL);                  check("%s NULL", "%s", "(null)", 0);
	begin(1); putPointer(2, "hi");                  check("%s text", "%s", "hi", 0);
	begin(1); putPointer(2, NULL);                  check("%S NULL", "%S", "(null)", 0);
	begin(1); putPointer(2, wide);                  check("%S text", "%S", "hi", 0);
	begin(1); putCount(4); putPointer(3, NULL);     check("%P NULL", "%P", "(null)", 0);
	begin(1); putCount(0); putPointer(3, NULL);     check("%P empty NULL", "%P", "<>", 0);
	begin(1); putCount(3); putPointer(3, bytes);    check("%P bytes", "%P", "<010203>", 0);
	begin(1); putPointer(4, &plain);                check("%@ nil description", "%@", "<Stub: ", 1);
	begin(1); putPointer(4, &described);            check("%@ unconvertible description", "%@", "<Stub: ", 1);

	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
