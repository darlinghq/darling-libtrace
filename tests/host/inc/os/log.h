#pragma once
// Just enough of Darling's os/log.h for the real os/log_private.h to parse on a plain host.
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>
#define OS_EXPORT __attribute__((visibility("default")))
#define OS_NOTHROW
#define OS_NOT_TAIL_CALLED
#define OS_NONNULL5
#define OS_ENUM(name, type, ...) typedef enum : type { __VA_ARGS__ } name##_t
typedef struct os_log_s *os_log_t;
typedef uint8_t os_log_type_t;
