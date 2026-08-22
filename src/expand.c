#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "expand.h"

#define EXPAND_SIZE 1024

static void expand_string(char *dest, size_t size, const char *src)
{
    size_t d = 0;
    size_t i = 0;

    while (src[i] != '\0' && d < size - 1)
    {
        /*
         * Normal character
         */
        if (src[i] != '$')
        {
            dest[d++] = src[i++];
            continue;
        }

        /*
         * $ at end
         */
        if (src[i + 1] == '\0')
        {
            dest[d++] = '$';
            i++;
            continue;
        }

        /*
         * ${VARIABLE}
         */
        if (src[i + 1] == '{')
        {
            size_t start = i + 2;
            size_t end = start;

            while (src[end] != '\0' &&
                   src[end] != '}')
            {
                end++;
            }

            if (src[end] == '}')
            {
                char name[256];

                size_t len = end - start;

                if (len >= sizeof(name))
                    len = sizeof(name) - 1;

                strncpy(name, src + start, len);
                name[len] = '\0';

                const char *value = getenv(name);

                if (value != NULL)
                {
                    size_t vlen = strlen(value);

                    for (size_t j = 0;
                         j < vlen && d < size - 1;
                         j++)
                    {
                        dest[d++] = value[j];
                    }
                }

                i = end + 1;
                continue;
            }
        }

        /*
         * $VARIABLE
         */
        if (isalpha((unsigned char)src[i + 1]) ||
            src[i + 1] == '_')
        {
            char name[256];

            size_t n = 0;
            size_t j = i + 1;

            while (src[j] != '\0' &&
                   (isalnum((unsigned char)src[j]) ||
                    src[j] == '_'))
            {
                if (n < sizeof(name) - 1)
                {
                    name[n++] = src[j];
                }

                j++;
            }

            name[n] = '\0';

            const char *value = getenv(name);

            if (value != NULL)
            {
                size_t vlen = strlen(value);

                for (size_t k = 0;
                     k < vlen && d < size - 1;
                     k++)
                {
                    dest[d++] = value[k];
                }
            }

            i = j;
            continue;
        }

        /*
         * If $ isn't followed by a variable name,
         * keep the $.
         */
        dest[d++] = '$';
        i++;
    }

    dest[d] = '\0';
}

void expand_variables(pipeline_t *pipeline)
{
    if (pipeline == NULL)
        return;

    for (int i = 0;
         i < pipeline->command_count;
         i++)
    {
        command_t *cmd = &pipeline->commands[i];

        for (int j = 0; j < cmd->argc; j++)
        {
            char expanded[EXPAND_SIZE];

            expand_string(expanded,
                          sizeof(expanded),
                          cmd->argv[j]);

            free(cmd->argv[j]);

            cmd->argv[j] = malloc(strlen(expanded) + 1);

            if (cmd->argv[j] != NULL)
            {
                strcpy(cmd->argv[j], expanded);
            }
        }

        if (cmd->input != NULL)
        {
            char expanded[EXPAND_SIZE];

            expand_string(expanded,
                          sizeof(expanded),
                          cmd->input);

            free(cmd->input);

            cmd->input = malloc(strlen(expanded) + 1);

            if (cmd->input != NULL)
            {
                strcpy(cmd->input, expanded);
            }
        }

        if (cmd->output != NULL)
        {
            char expanded[EXPAND_SIZE];

            expand_string(expanded,
                          sizeof(expanded),
                          cmd->output);

            free(cmd->output);

            cmd->output = malloc(strlen(expanded) + 1);

            if (cmd->output != NULL)
            {
                strcpy(cmd->output, expanded);
            }
        }
    }
}
