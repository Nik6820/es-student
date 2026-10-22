#pragma once

#include <stdint.h> 

#define DEVICE_NAME "231-firmware-template"
#define FIRMWARE_VERSION "1.0.0"

#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/<Nik6820>/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif


struct info_t
{
    uint32_t version;    // 4 байта — первым
    char name[13];       // 13 байт — посередине
    uint8_t revision;    // 1 байт — в хвост
};

void device_info(void); 


