#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "token.h"

void command_init(command_t *cmd)
{
    if (cmd == NULL)
        return;

    cmd->argc = 0;
    cmd->input = NULL;
    cmd->output = NULL;
    cmd->append = 0;
    cmd->background = 0;

    for (int i = 0; i < MAX_ARGS; i++)
    {
        cmd->argv[i] = NULL;
    }
}

void pipeline_init(pipeline_t *pipeline)
{
    if (pipeline == NULL)
        return;

    pipeline->command_count = 0;

    for (int i = 0; i < MAX_COMMANDS; i++)
    {
        command_init(&pipeline->commands[i]);
    }
}

static char *duplicate_string(const char *str)
{
    if (str == NULL)
        return NULL;

    char *copy = malloc(strlen(str) + 1);

    if (copy == NULL)
        return NULL;

    strcpy(copy, str);

    return copy;
}

int parse(const token_list_t *tokens, pipeline_t *pipeline)
{
    if (tokens == NULL || pipeline == NULL)
        return 0;

    pipeline_init(pipeline);

    pipeline->command_count = 1;

    int current = 0;

    for (int i = 0; i < tokens->count; i++)
    {
        token_t token = tokens->tokens[i];

        /*
         * End of command
         */
        if (token.type == TOKEN_END)
        {
            break;
        }

        /*
         * WORD
         */
        if (token.type == TOKEN_WORD)
        {
            if (pipeline->commands[current].argc >= MAX_ARGS - 1)
            {
                printf("Error: too many arguments\n");
                return 0;
            }

            pipeline->commands[current]
                .argv[pipeline->commands[current].argc] =
                    duplicate_string(token.text);

            pipeline->commands[current].argc++;

            continue;
        }

        /*
         * Input redirection <
         */
        if (token.type == TOKEN_INPUT)
        {
            if (i + 1 >= tokens->count ||
                tokens->tokens[i + 1].type != TOKEN_WORD)
            {
                printf("Error: filename expected after <\n");
                return 0;
            }

            pipeline->commands[current].input =
                duplicate_string(tokens->tokens[i + 1].text);

            i++;

            continue;
        }

        /*
         * Output redirection >
         */
        if (token.type == TOKEN_OUTPUT)
        {
            if (i + 1 >= tokens->count ||
                tokens->tokens[i + 1].type != TOKEN_WORD)
            {
                printf("Error: filename expected after >\n");
                return 0;
            }

            pipeline->commands[current].output =
                duplicate_string(tokens->tokens[i + 1].text);

            pipeline->commands[current].append = 0;

            i++;

            continue;
        }

        /*
         * Append >>
         */
        if (token.type == TOKEN_APPEND)
        {
            if (i + 1 >= tokens->count ||
                tokens->tokens[i + 1].type != TOKEN_WORD)
            {
                printf("Error: filename expected after >>\n");
                return 0;
            }

            pipeline->commands[current].output =
                duplicate_string(tokens->tokens[i + 1].text);

            pipeline->commands[current].append = 1;

            i++;

            continue;
        }

        /*
         * Background &
         */
        if (token.type == TOKEN_BACKGROUND)
        {
            pipeline->commands[current].background = 1;

            continue;
        }

        /*
         * Pipe |
         */
        if (token.type == TOKEN_PIPE)
        {
            if (pipeline->commands[current].argc == 0)
            {
                printf("Error: empty command before pipe\n");
                return 0;
            }

            current++;

            if (current >= MAX_COMMANDS)
            {
                printf("Error: too many commands in pipeline\n");
                return 0;
            }

            command_init(&pipeline->commands[current]);

            pipeline->command_count++;

            continue;
        }
    }

    /*
     * Every command needs at least one argument.
     */
    for (int i = 0; i < pipeline->command_count; i++)
    {
        if (pipeline->commands[i].argc == 0)
        {
            printf("Error: empty command\n");
            return 0;
        }
    }

    return 1;
}

void pipeline_print(const pipeline_t *pipeline)
{
    if (pipeline == NULL)
        return;

    printf("\n========== PIPELINE ==========\n");

    for (int i = 0; i < pipeline->command_count; i++)
    {
        const command_t *cmd = &pipeline->commands[i];

        printf("\nCommand %d\n", i + 1);
        printf("------------------------------\n");

        printf("Arguments\n");

        for (int j = 0; j < cmd->argc; j++)
        {
            printf("argv[%d] = %s\n", j, cmd->argv[j]);
        }

        if (cmd->input != NULL)
            printf("Input    : %s\n", cmd->input);
        else
            printf("Input    : None\n");

        if (cmd->output != NULL)
            printf("Output   : %s\n", cmd->output);
        else
            printf("Output   : None\n");

        printf("Append   : %s\n",
               cmd->append ? "Yes" : "No");

        printf("Background : %s\n",
               cmd->background ? "Yes" : "No");
    }

    printf("==============================\n");
}

void pipeline_free(pipeline_t *pipeline)
{
    if (pipeline == NULL)
        return;

    for (int i = 0; i < pipeline->command_count; i++)
    {
        command_t *cmd = &pipeline->commands[i];

        for (int j = 0; j < cmd->argc; j++)
        {
            free(cmd->argv[j]);
            cmd->argv[j] = NULL;
        }

        free(cmd->input);
        free(cmd->output);

        cmd->input = NULL;
        cmd->output = NULL;
    }

    pipeline->command_count = 0;
}
