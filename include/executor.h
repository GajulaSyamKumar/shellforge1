#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

/*
 * Execute a single command.
 *
 * Built-in commands are executed directly.
 * External commands use fork(), execvp(), waitpid().
 */
int execute_command(const command_t *cmd);


/*
 * Execute a complete pipeline.
 *
 * Example:
 *
 *     ls | grep .c
 *
 *     ls -l | grep .c | wc -l
 *
 * Uses:
 *     pipe()
 *     fork()
 *     dup2()
 *     execvp()
 *     waitpid()
 */
int execute_pipeline(const pipeline_t *pipeline);

#endif
