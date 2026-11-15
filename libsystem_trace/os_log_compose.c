#include <os/log_private.h>

// There is no log backend and no formatter here, so nothing can be composed or sent. The callers (libc's
// _os_crash_fmt_impl, configd's SCLog) print, syslog and free() the result: they must see NULL, not whatever
// a void stub leaves in x0.

char *
os_log_pack_compose(os_log_pack_t pack, os_log_t log, os_log_type_t type, char *buffer, size_t buffer_size)
{
	return NULL;
}

char *
os_log_pack_send_and_compose(os_log_pack_t pack, os_log_t log, os_log_type_t type, char *buffer, size_t buffer_size)
{
	return NULL;
}
