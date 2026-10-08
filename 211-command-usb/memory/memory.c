#include "memory.h"
#include "command.h"
#include "device.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/regs/addressmap.h"

int main(void);

uint32_t data_variable = 100;

uint32_t bss_variable;

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name,
           (unsigned)start,
           (unsigned)end,
           (unsigned)(end - start));
}

void mem_info(void)
{
    const uintptr_t flash_start = XIP_BASE;
    const uintptr_t flash_end   = XIP_BASE + PICO_FLASH_SIZE_BYTES;
    const uintptr_t sram_start  = SRAM_BASE;
    const uintptr_t sram_end    = SRAM_BASE + 264u * 1024u;   /* 264 КБ SRAM */
    const uintptr_t rom_start   = ROM_BASE;
    const uintptr_t rom_end     = ROM_BASE + 16u * 1024u;      /* 16 КБ Boot ROM */

    const uintptr_t image_start    = (uintptr_t)&__flash_binary_start;
    const uintptr_t image_end      = (uintptr_t)&__flash_binary_end;
    const uintptr_t boot2_start    = (uintptr_t)&__boot2_start__;
    const uintptr_t boot2_end      = (uintptr_t)&__boot2_end__;
    const uintptr_t etext          = (uintptr_t)&__etext;
    const uintptr_t data_ram_start = (uintptr_t)&__data_start__;
    const uintptr_t data_ram_end   = (uintptr_t)&__data_end__;
    const uintptr_t bss_start      = (uintptr_t)&__bss_start__;
    const uintptr_t bss_end        = (uintptr_t)&__bss_end__;
    const uintptr_t heap_end       = (uintptr_t)&__HeapLimit;
    const uintptr_t stack_bottom   = (uintptr_t)&__StackBottom;
    const uintptr_t stack_top      = (uintptr_t)&__StackTop;

    const uintptr_t data_size  = data_ram_end - data_ram_start;
    const uintptr_t boot2_size = boot2_end - boot2_start;
    const uintptr_t text_size  = etext - boot2_end;
    const uintptr_t bss_size   = bss_end - bss_start;
    const uintptr_t heap_size  = heap_end - bss_end;
    const uintptr_t stack_size = stack_top - stack_bottom;

    printf("area       start      end        size\n");
    row("flash",      flash_start,    flash_end);
    row("sram",       sram_start,     sram_end);
    row("rom",        rom_start,      rom_end);
    row("image",      image_start,    image_end);
    row("free",       image_end,      flash_end);
    row("boot2",      boot2_start,    boot2_end);
    row("text",       boot2_end,      etext);
    row("data flash", etext,          etext + data_size); /* .data во флеш */
    row("data ram",   data_ram_start, data_ram_end);      /* .data в ОЗУ   */
    row("bss",        bss_start,      bss_end);
    row("heap",       bss_end,        heap_end);
    row("stack",      stack_bottom,   stack_top);

    printf("\n");
    printf("total\n");
    printf("  %-12s%8u = boot2 %u + text %u + data %u\n",
           "flash image",
           (unsigned)(image_end - image_start),
           (unsigned)boot2_size,
           (unsigned)text_size,
           (unsigned)data_size);
    printf("  %-12s%8u of %u\n",
           "flash free",
           (unsigned)(flash_end - image_end),
           (unsigned)PICO_FLASH_SIZE_BYTES);
    printf("  %-12s%8u = data %u + bss %u\n",
           "ram used",
           (unsigned)(data_size + bss_size),
           (unsigned)data_size,
           (unsigned)bss_size);
    printf("  %-12s%8u for heap and %u for stack\n",
           "ram free",
           (unsigned)heap_size,
           (unsigned)stack_size);
}

void fw_info(void)
{
    data_variable++;
    bss_variable++;

    uint32_t stack_variable = 1946;

    uint32_t *heap_variable = (uint32_t *)malloc(sizeof(uint32_t));
    if (heap_variable != NULL)
    {
        *heap_variable = 1951;
    }

    uint16_t *main_code    = (uint16_t *)((uintptr_t)main & ~(uintptr_t)1u);
    uint16_t *fw_info_code = (uint16_t *)((uintptr_t)fw_info & ~(uintptr_t)1u);

    printf("object          address     value\n");

    printf("main            0x%08x  0x%04x\n",
           (unsigned)(uintptr_t)main,
           (unsigned)*main_code);

    printf("fw_info         0x%08x  0x%04x\n",
           (unsigned)(uintptr_t)fw_info,
           (unsigned)*fw_info_code);

    printf("commands        0x%08x\n",
           (unsigned)(uintptr_t)commands);

    for (uint i = 0; i < command_count; i++)
    {
        printf("- %-12s 0x%08x\n",
               commands[i].name,
               (unsigned)(uintptr_t)commands[i].handler);
    }

    printf("DEVICE_PROJECT  0x%08x  %s\n",
           (unsigned)(uintptr_t)DEVICE_PROJECT,
           DEVICE_PROJECT);

    printf("DEVICE_BOARD    0x%08x  %s\n",
           (unsigned)(uintptr_t)DEVICE_BOARD,
           DEVICE_BOARD);

    printf("data_variable   0x%08x  %u\n",
           (unsigned)(uintptr_t)&data_variable,
           (unsigned)data_variable);

    printf("bss_variable    0x%08x  %u\n",
           (unsigned)(uintptr_t)&bss_variable,
           (unsigned)bss_variable);

    printf("stack_variable  0x%08x  %u\n",
           (unsigned)(uintptr_t)&stack_variable,
           (unsigned)stack_variable);

    printf("heap_variable   0x%08x  %u\n",
           (unsigned)(uintptr_t)heap_variable,
           heap_variable != NULL ? (unsigned)*heap_variable : 0u);

    free(heap_variable);
}