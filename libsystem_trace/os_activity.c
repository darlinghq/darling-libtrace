#include <os/activity.h>

void _os_activity_initiate(void *dso, const char *description, os_activity_flag_t flags, os_block_t activity_block OS_NOESCAPE) {
	// TODO: make this actually work like Apple's _os_activity_initiate
	// this is pretty much just a stub for now that allows the block to execute
	activity_block();
};