project(libsystem_trace)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libsbuf)

add_definitions(-mmacosx-version-min=10.13)

set(system_trace_sources
	libsystem_trace/libtrace_assert.c
	libsystem_trace/os_log.c
	libsystem_trace/os_activity.c
	libsystem_trace/stubs.c
	libsystem_trace/unpack_buffer.c
	libsystem_trace/classes.m
	libsbuf/libsbuf.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_trace.dylib")
add_circular(system_trace FAT
	SOURCES
		${system_trace_sources}
	SIBLINGS
		platform system_kernel system_c system_malloc system_asl system_duct libdispatch_shared system_dyld
	UPWARD
		objc
	LINK_FLAGS
		"-mmacosx-version-min=10.13"
)

install(TARGETS system_trace DESTINATION libexec/darling/usr/lib/system)
install(FILES libsystem_trace/org.puredarwin.os_log.asl DESTINATION libexec/darling/etc/asl)
