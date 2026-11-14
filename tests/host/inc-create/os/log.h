#pragma once
#include <stdint.h>
typedef struct os_log_s *os_log_t;
os_log_t os_log_create(const char *subsystem, const char *category);
