#include <os/log_private.h>

// Declared in <mach/mach_time.h> only under PRIVATE; libsystem_kernel exports it.
int mach_get_times(uint64_t *absolute_time, uint64_t *continuous_time, struct timespec *tp);

// Callers (CoreFoundation's os_log helpers, for one) size a buffer with _os_log_pack_size, hand it to
// _os_log_pack_fill and write their encoded arguments through the returned pointer, so the pointer
// must lead into that buffer.

size_t
_os_log_pack_size(size_t os_log_format_buffer_size)
{
	return sizeof(struct os_log_pack_s) + os_log_format_buffer_size;
}

// saved_errno is not stored: the pack has no field for it. This build has no log backend to read it back.
uint8_t *
_os_log_pack_fill(os_log_pack_t pack, size_t size, int saved_errno, const void *dso, const char *format)
{
	(void)size;
	(void)saved_errno;
	mach_get_times(NULL, &pack->olp_continuous_time, &pack->olp_wall_time);
	pack->olp_mh = dso;
	pack->olp_pc = __builtin_return_address(0);
	pack->olp_format = format;
	return pack->olp_data;
}
