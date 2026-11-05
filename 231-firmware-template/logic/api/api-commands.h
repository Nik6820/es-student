#pragma once

#include <stdint.h>
#include "api/api.h"            // берём command_t и api_handler_t отсюда

typedef struct {
    const char *name;
    api_handler_t handler;
} api_command_t;

extern const api_command_t api_commands[];
extern const uint32_t api_command_count;