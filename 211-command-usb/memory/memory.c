#include "memory.h"

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/regs/addressmap.h"

/* Символы линкера — это метки адресов, а не объекты.
   Берём только их адрес через &, значение читать нельзя. */
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
    /* Границы регионов чипа — из SDK и datasheet */
    const uintptr_t flash_start = XIP_BASE;
    const uintptr_t flash_end   = XIP_BASE + PICO_FLASH_SIZE_BYTES;
    const uintptr_t sram_start  = SRAM_BASE;
    const uintptr_t sram_end    = SRAM_BASE + 264u * 1024u;   /* 264 КБ SRAM */
    const uintptr_t rom_start   = ROM_BASE;
    const uintptr_t rom_end     = ROM_BASE + 16u * 1024u;      /* 16 КБ Boot ROM */

    /* Границы образа — из символов линкера */
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

    /* Размеры, которые встретятся не один раз */
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