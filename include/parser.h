#ifndef PARSER_H
#define PARSER_H

#include "token.h"

#define MAX_ARGS 128
#define MAX_COMMANDS 16

typedef struct
{
    char *argv[MAX_ARGS];

    char *input;
    char *output;

    int append;
    int background;

    int argc;

} command_t;

typedef struct
{
    command_t commands[MAX_COMMANDS];

    int command_count;

} pipeline_t;

void command_init(command_t *cmd);

void pipeline_init(pipeline_t *pipeline);

int parse(const token_list_t *tokens, pipeline_t *pipeline);

void pipeline_print(const pipeline_t *pipeline);

void pipeline_free(pipeline_t *pipeline);

#endif
