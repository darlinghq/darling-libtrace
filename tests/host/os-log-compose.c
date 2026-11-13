#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os/log_private.h>

// os_log_pack_compose and os_log_pack_send_and_compose return a composed string. Callers print it, pass it to
// abort_with_payload or free() it unless it is their own buffer, so a stub must return NULL, not stray register
// contents. Called through the prototype libc's assumes.c uses.

static int failures;
#define CHECK(cond) do { if (!(cond)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

typedef char *(*compose_fn)(os_log_pack_t, os_log_t, os_log_type_t, char *, size_t);

int main(void)
{
	static struct os_log_pack_s pack;
	char buffer[256];
	compose_fn fns[] = { os_log_pack_compose, os_log_pack_send_and_compose };
	const char *names[] = { "os_log_pack_compose", "os_log_pack_send_and_compose" };

	for (int i = 0; i < 2; i++) {
		memset(buffer, 'x', sizeof(buffer));
		char *composed = fns[i](&pack, NULL, 0, buffer, sizeof(buffer));
		if (composed != NULL) fprintf(stderr, "FAIL %s returned non-NULL\n", names[i]);
		CHECK(composed == NULL);
		// libc's call shape: no buffer at all
		CHECK(fns[i](&pack, NULL, 0, NULL, 0) == NULL);
		// configd's cleanup: free unless it is the caller's own buffer; NULL must be safe
		if (composed != buffer) free(composed);
		CHECK(buffer[0] == 'x');
	}
	if (failures) return 1;
	puts("os-log-compose: ok");
	return 0;
}
