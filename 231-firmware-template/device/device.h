#pragma once

#include <stdint.h>

#define DEVICE_NAME      "231-firmware-template"
#define FIRMWARE_VERSION "1.0.0"
#define DEVICE_PROJECT   "231-firmware-template"
#define DEVICE_REPO      "https://github.com/Nik6820/es-student"

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