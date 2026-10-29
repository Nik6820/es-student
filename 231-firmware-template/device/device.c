#include "device.h"

#include "pico/stdlib.h"
#include "pico/version.h"
#include "pico/unique_id.h"
#include <string.h>

void device_get_info(device_info_t *info)
{
    if (info == NULL) return;

    /* Тип платы */
    strncpy(info->board, DEVICE_BOARD, sizeof(info->board) - 1);
    info->board[sizeof(info->board) - 1] = '\0';

    /* Серийный номер: 16 hex-цифр + \0 */
    pico_get_unique_board_id_string(info->serial, sizeof(info->serial));

    /* CHIP_ID: производитель 3 hex, изделие 4 hex, ревизия dec */
    uint32_t chip_id   = *(volatile uint32_t *)0x40000000u;
    info->manufacturer =  chip_id         & 0xFFFu;
    info->part         = (chip_id >> 12)  & 0xFFFFu;
    info->revision     = (chip_id >> 28)  & 0xFu;

    /* Версия SDK */
    strncpy(info->sdk_version, PICO_SDK_VERSION_STRING,
            sizeof(info->sdk_version) - 1);
    info->sdk_version[sizeof(info->sdk_version) - 1] = '\0';
}