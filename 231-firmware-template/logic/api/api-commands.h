#pragma once

#include <stdint.h>
#include "api/api.h"            // command_t и api_handler_t

typedef struct {
    const char *name;
    const char *help;
    api_handler_t callback;     // ← было handler
} api_command_t;

extern const api_command_t api_commands[];
extern const uint32_t api_command_count;