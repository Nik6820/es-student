#pragma once

#include <stdint.h>

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

typedef struct
{
    char     board[32];
    char     serial[17];
    unsigned manufacturer;
    unsigned part;
    unsigned revision;
    char     sdk_version[16];
} device_info_t;

void device_get_info(device_info_t *info);