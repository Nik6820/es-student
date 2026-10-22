#include "stdio-text-protocol.h"
#include "pico/stdlib.h"
#include <stdio.h>

static char buffer[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint32_t length;
static bool truncated;
static command_t command;

void stdio_text_protocol_init(void)
{
    length = 0;
    truncated = false;
    buffer[0] = '\0';
}

const command_t *stdio_text_protocol_handle(void)
{
    int c = getchar_timeout_us(0);
    if (c == PICO_ERROR_TIMEOUT)
    {
        return NULL;
    }

    /* Конец строки */
    if (c == '\r' || c == '\n')
    {
        if (length == 0)
        {
            return NULL;
        }

        putchar('\r');
        putchar('\n');

        buffer[length] = '\0';

        command.truncated = truncated;
        command.argc = 0;
        command.name = NULL;
        for (uint32_t i = 0; i < STDIO_TEXT_PROTOCOL_MAX_ARGS; i++)
        {
            command.argv[i] = NULL;
        }

        uint32_t i = 0;
        while (i < length && buffer[i] == ' ') i++;

        if (i == length)
        {
            /* кадр из одних пробелов — командой не считается */
            length = 0;
            truncated = false;
            return NULL;
        }

        uint32_t start = i;
        while (i < length && buffer[i] != ' ') i++;
        buffer[i] = '\0';
        command.name = &buffer[start];
        i++;

        while (i < length)
        {
            while (i < length && buffer[i] == ' ') i++;
            if (i >= length) break;

            start = i;
            while (i < length && buffer[i] != ' ') i++;
            buffer[i] = '\0';

            if (command.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS)
            {
                command.argv[command.argc++] = &buffer[start];
            }
            else
            {
                command.truncated = true;
            }

            i++;
        }

        length = 0;
        truncated = false;
        return &command;
    }

    /* Backspace */
    if (c == 0x08 || c == 0x7f)
    {
        if (length > 0)
        {
            length--;
            truncated = false;
            putchar(0x08);
            putchar(0x20);
            putchar(0x08);
        }
        return NULL;
    }

    /* Печатный символ */
    if (c >= 0x20 && c <= 0x7e)
    {
        if (length < STDIO_TEXT_PROTOCOL_LINE_SIZE - 1)
        {
            buffer[length++] = (char)c;
            putchar(c);
        }
        else
        {
            truncated = true;
        }
    }

    return NULL;
}