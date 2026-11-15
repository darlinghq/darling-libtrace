#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <os/log_private.h>

// A caller sizes a buffer with _os_log_pack_size, fills the header with _os_log_pack_fill and writes its
// encoded arguments through the returned pointer. The pointer must lie inside that buffer.

#define FORMAT_BUFFER_SIZE 24
#define CONTINUOUS_TIME 0x1122334455667788ULL
#define WALL_SEC 1700000000
#define WALL_NSEC 123456789

int mach_get_times(uint64_t *absolute_time, uint64_t *continuous_time, struct timespec *tp)
{
	if (continuous_time) *continuous_time = CONTINUOUS_TIME;
	if (tp) { tp->tv_sec = WALL_SEC; tp->tv_nsec = WALL_NSEC; }
	return 0;
}

static int failures;
#define CHECK(cond) do { if (!(cond)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static int dso_marker;

int main(void)
{
	size_t size = _os_log_pack_size(FORMAT_BUFFER_SIZE);
	CHECK(size == sizeof(struct os_log_pack_s) + FORMAT_BUFFER_SIZE);

	// Exactly sized: a write past the returned buffer is a heap overflow that a sanitizer reports.
	os_log_pack_t pack = malloc(size);
	assert(pack);
	// Zero first: a header field fill forgets to set would otherwise hold allocator fill, not a checkable 0.
	memset(pack, 0, size);
	uint8_t *data = _os_log_pack_fill(pack, size, 0, &dso_marker, "%s");

	CHECK(data != NULL);
	if (data) {
		CHECK(data == (uint8_t *)pack + offsetof(struct os_log_pack_s, olp_data));
		memset(data, 0xA5, FORMAT_BUFFER_SIZE);
		CHECK(data[FORMAT_BUFFER_SIZE - 1] == 0xA5);
	}
	CHECK(pack->olp_continuous_time == CONTINUOUS_TIME);
	CHECK(pack->olp_wall_time.tv_sec == WALL_SEC && pack->olp_wall_time.tv_nsec == WALL_NSEC);
	CHECK(pack->olp_mh == &dso_marker);
	CHECK(pack->olp_pc != NULL);
	CHECK(strcmp(pack->olp_format, "%s") == 0);
	CHECK(_os_log_pack_size(0) == sizeof(struct os_log_pack_s));
	free(pack);

	if (failures) return 1;
	puts("os-log-pack: ok");
	return 0;
}
