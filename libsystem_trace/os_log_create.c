/*
 * Copyright (c) 2019 PureDarwin Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <os/lock.h>
#include <stdlib.h>
#include <string.h>
#include "os_log_s.h"
#include "libtrace_assert.h"
#include <os/log.h>
#import <os/object_private.h>

OS_OBJECT_OBJC_CLASS_DECL(os_log);

// os_log_create returns +1 and the caller may release it. Messages' frameworks release once more than they retain
// (a final release, then a retain: libdispatch aborted with "Resurrection of an object"), so a log object is created
// once per subsystem and category and is immortal, which tolerates any balance. GUESS, NEEDS-REAL-MAC.
struct os_log_node {
	struct os_log_node *next;
	os_log_t log;
};
static os_unfair_lock s_logs_lock = OS_UNFAIR_LOCK_INIT;
static struct os_log_node *s_logs;

// The caller holds s_logs_lock.
static os_log_t find_log(const char *subsystem, const char *category) {
	for (struct os_log_node *node = s_logs; node != NULL; node = node->next) {
		if (strcmp(node->log->subsystem, subsystem) == 0 && strcmp(node->log->category, category) == 0)
			return node->log;
	}
	return NULL;
}

os_log_t os_log_create(const char *subsystem, const char *category) {
	libtrace_precondition(subsystem != NULL, "subsystem cannot be NULL");
	libtrace_precondition(category != NULL, "category cannot be NULL");

	os_unfair_lock_lock(&s_logs_lock);
	os_log_t found = find_log(subsystem, category);
	os_unfair_lock_unlock(&s_logs_lock);
	if (found != NULL)
		return found;

	os_log_t value = (os_log_t)_os_object_alloc(&OS_OBJECT_CLASS_SYMBOL(os_log), sizeof(struct os_log_s));
	value->magic = OS_LOG_MAGIC;
	value->subsystem = strdup(subsystem);
	value->category = strdup(category);
	libtrace_assert(value->subsystem != NULL && value->category != NULL, "out of memory creating a log object");
	struct os_log_node *node = malloc(sizeof(*node));
	libtrace_assert(node != NULL, "out of memory creating a log object");

	os_unfair_lock_lock(&s_logs_lock);
	found = find_log(subsystem, category);
	if (found == NULL) {
		// Immortal only once published: until then a release disposes the spare below.
		value->ref_cnt = _OS_OBJECT_GLOBAL_REFCNT;
		value->xref_cnt = _OS_OBJECT_GLOBAL_REFCNT;
		node->log = value;
		node->next = s_logs;
		s_logs = node;
	}
	os_unfair_lock_unlock(&s_logs_lock);
	if (found != NULL) {
		// Another thread published the same log first; drop this spare.
		free(node);
		os_release(value);
		return found;
	}
	return value;
}
