#pragma once

#include "stdio-text-protocol/stdio-text-protocol.h"

// тип обработчика команды прибора: один на весь модуль
typedef void (*api_handler_t)(const command_t *command);

void api_handle(const command_t *command);