#include "api/api.h"            // свой заголовок первым (П10)
#include "api/api-commands.h"

#include <stdio.h>
#include <string.h>

// механизм: имена команд прибора здесь НЕ упоминаются
static const api_command_t *find_command(const char *name)
{
    if (name == NULL) {
        return NULL;
    }
    for (uint32_t i = 0; i < api_command_count; i++) {
        if (strcmp(api_commands[i].name, name) == 0) {
            return &api_commands[i];
        }
    }
    return NULL;
}

// help — служебный ответ механизма, поэтому живёт в api.c
static void reply_help(void)
{
    printf("commands:\n");
    for (uint32_t i = 0; i < api_command_count; i++) {
        printf("  %s\n", api_commands[i].name);
    }
}

void api_handle(const command_t *command)
{
    if (command == NULL) {
        return;
    }

    // ответ на обрезанный кадр
    if (command->truncated) {
        printf("error: command truncated\n");
        return;
    }

    if (command->name == NULL || command->name[0] == '\0') {
        printf("error: empty command\n");
        return;
    }

    // help — единственное имя команды, допустимое в этом файле
    if (strcmp(command->name, "help") == 0) {
        reply_help();
        return;
    }

    const api_command_t *cmd = find_command(command->name);
    if (cmd != NULL && cmd->handler != NULL) {
        cmd->handler(command);
    } else {
        printf("error: unknown command '%s'\n", command->name);
    }
}