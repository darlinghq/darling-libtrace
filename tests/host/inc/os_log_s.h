#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define OS_ENUM(name, type, ...) enum { __VA_ARGS__ }; typedef type name##_t
#define __XNU_PRIVATE_EXTERN
#ifndef __APPLE__
#define isnumber(c) isdigit((unsigned char)(c))
#endif
#include <os/base.h>
