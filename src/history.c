#include <stdio.h>
#include <string.h>

#include "history.h"

static char history[MAX_HISTORY][1024];

static int history_count = 0;

void history_init(void)
{
    history_count = 0;

    for (int i = 0; i < MAX_HISTORY; i++)
    {
        history[i][0] = '\0';
    }
}

void history_add(const char *command)
{
    if (command == NULL || command[0] == '\0')
        return;

    if (history_count >= MAX_HISTORY)
    {
        for (int i = 1; i < MAX_HISTORY; i++)
        {
            strcpy(history[i - 1], history[i]);
        }

        history_count = MAX_HISTORY - 1;
    }

    strncpy(history[history_count],
            command,
            sizeof(history[history_count]) - 1);

    history[history_count]
        [sizeof(history[history_count]) - 1] = '\0';

    history_count++;
}

void history_print(void)
{
    printf("-------- Command History --------\n");

    for (int i = 0; i < history_count; i++)
    {
        printf("%d %s\n", i + 1, history[i]);
    }

    printf("---------------------------------\n");
}
