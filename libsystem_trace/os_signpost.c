/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling Developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <os/signpost.h>
#include <stdatomic.h>
#include <stdbool.h>

// Signposts are not recorded, so no log handle has them enabled. Callers still
// get valid identifiers, because they may pass them around or compare them
// regardless of whether the signposts are emitted.

bool
os_signpost_enabled(os_log_t log)
{
	(void)log;
	return false;
}

static bool
is_reserved(os_signpost_id_t spid)
{
	return spid == OS_SIGNPOST_ID_NULL || spid == OS_SIGNPOST_ID_INVALID || spid == OS_SIGNPOST_ID_EXCLUSIVE;
}

os_signpost_id_t
os_signpost_id_generate(os_log_t log)
{
	static _Atomic os_signpost_id_t next = 1;
	os_signpost_id_t spid;

	(void)log;
	do {
		spid = atomic_fetch_add_explicit(&next, 1, memory_order_relaxed);
	} while (is_reserved(spid));
	return spid;
}

// The identifier depends only on the pointer, so that the code that begins an
// interval and the code that ends it can both derive it from the same object.
os_signpost_id_t
os_signpost_id_make_with_pointer(os_log_t log, const void *ptr)
{
	os_signpost_id_t spid = (os_signpost_id_t)(uintptr_t)ptr;

	(void)log;
	return is_reserved(spid) ? OS_SIGNPOST_ID_INVALID : spid;
}
