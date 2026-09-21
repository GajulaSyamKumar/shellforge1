#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"

/*
 * Check whether a command is a Shellforge built-in.
 */
int is_builtin(const command_t *cmd);

/*
 * Execute a Shellforge built-in.
 *
 * Return:
 *     0  -> normal
 *     1  -> exit shell
 *    -1  -> error
 */
int execute_builtin(const command_t *cmd);

#endif
