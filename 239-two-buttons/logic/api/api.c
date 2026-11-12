#include "api/api.h"
#include "api/api-commands.h"

#include <stdio.h>
#include <string.h>

static const api_command_t *find_command(const char *name)
{
    if (name == NULL) return NULL;

    for (uint32_t i = 0; i < api_command_count; i++)
    {
        if (strcmp(api_commands[i].name, name) == 0)
        {
            return &api_commands[i];
        }
    }
    return NULL;
}

void api_handle(const command_t *command)
{
    if (command == NULL) return;

    if (command->truncated)
    {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }

    if (strcmp(command->name, "help") == 0)
    {
        printf("%-16s %s\n", "help", "list of commands");
        for (uint32_t i = 0; i < api_command_count; i++)
        {
            printf("%-16s %s\n", api_commands[i].name, api_commands[i].help);
        }
        return;
    }

    const api_command_t *cmd = find_command(command->name);
    if (cmd != NULL && cmd->callback != NULL)
    {
        cmd->callback(command);
        return;
    }

    printf("error: unknown command '%s', try help\n", command->name);
}