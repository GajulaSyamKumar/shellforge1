#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "executor.h"
#include "builtin.h"


/*
 * ============================================================
 * EXECUTE EXTERNAL COMMAND
 * ============================================================
 */
static int execute_external(const command_t *cmd)
{
    pid_t pid;
    int status;


    if (cmd == NULL || cmd->argc <= 0)
    {
        return -1;
    }


    /*
     * Create child.
     */
    pid = fork();


    if (pid < 0)
    {
        perror("fork");
        return -1;
    }


    /*
     * ========================================================
     * CHILD
     * ========================================================
     */
    if (pid == 0)
    {
        /*
         * Execute external program.
         */
        execvp(
            cmd->argv[0],
            cmd->argv
        );


        /*
         * execvp() returns only if it fails.
         */
        perror(cmd->argv[0]);

        _exit(127);
    }


    /*
     * ========================================================
     * PARENT
     * ========================================================
     */
    if (waitpid(
            pid,
            &status,
            0
        ) < 0)
    {
        perror("waitpid");
        return -1;
    }


    /*
     * Return child's exit status.
     */
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }


    /*
     * Process terminated by signal.
     */
    if (WIFSIGNALED(status))
    {
        return 128 + WTERMSIG(status);
    }


    return -1;
}


/*
 * ============================================================
 * EXECUTE SINGLE COMMAND
 * ============================================================
 */
int execute_command(const command_t *cmd)
{
    if (cmd == NULL || cmd->argc <= 0)
    {
        return -1;
    }


    /*
     * Built-in command.
     *
     * IMPORTANT:
     *
     * cd must execute in the shell process.
     */
    if (is_builtin(cmd))
    {
        return execute_builtin(cmd);
    }


    /*
     * External command.
     */
    return execute_external(cmd);
}


/*
 * ============================================================
 * EXECUTE PIPELINE
 *
 * Example:
 *
 *     ls | grep .c
 *
 * Data flow:
 *
 *     ls
 *      |
 *      v
 *    pipe 1
 *      |
 *      v
 *    grep
 *
 *
 * Three commands:
 *
 *     ls | grep .c | wc -l
 *
 *     ls
 *      |
 *    pipe1
 *      |
 *    grep
 *      |
 *    pipe2
 *      |
 *    wc
 *
 * ============================================================
 */
int execute_pipeline(const pipeline_t *pipeline)
{
    int command_count;
    int previous_read = -1;

    pid_t *pids;

    int last_status = 0;

    int i;


    /*
     * ========================================================
     * VALIDATION
     * ========================================================
     */
    if (pipeline == NULL)
    {
        return -1;
    }


    command_count = pipeline->command_count;


    if (command_count <= 0)
    {
        return -1;
    }


    /*
     * ========================================================
     * ONLY ONE COMMAND
     *
     * For a single command, use execute_command().
     * ========================================================
     */
    if (command_count == 1)
    {
        return execute_command(
            &pipeline->commands[0]
        );
    }


    /*
     * ========================================================
     * Allocate PID array.
     *
     * One PID for each command.
     * ========================================================
     */
    pids = malloc(
        sizeof(pid_t) * command_count
    );


    if (pids == NULL)
    {
        perror("malloc");
        return -1;
    }


    /*
     * ========================================================
     * CREATE EACH PIPE AND CHILD
     * ========================================================
     */
    for (i = 0; i < command_count; i++)
    {
        int pipefd[2];


        /*
         * ----------------------------------------------------
         * Create pipe for every command except the last.
         * ----------------------------------------------------
         */
        if (i < command_count - 1)
        {
            if (pipe(pipefd) < 0)
            {
                perror("pipe");

                /*
                 * Close previous pipe if open.
                 */
                if (previous_read != -1)
                {
                    close(previous_read);
                }

                free(pids);

                return -1;
            }
        }


        /*
         * ----------------------------------------------------
         * Create child.
         * ----------------------------------------------------
         */
        pids[i] = fork();


        if (pids[i] < 0)
        {
            perror("fork");

            if (previous_read != -1)
            {
                close(previous_read);
            }


            if (i < command_count - 1)
            {
                close(pipefd[0]);
                close(pipefd[1]);
            }


            free(pids);

            return -1;
        }


        /*
         * ====================================================
         * CHILD PROCESS
         * ====================================================
         */
        if (pids[i] == 0)
        {
            /*
             * ------------------------------------------------
             * INPUT FROM PREVIOUS PIPE
             *
             * For commands after the first:
             *
             *     previous_read -> STDIN
             * ------------------------------------------------
             */
            if (previous_read != -1)
            {
                if (dup2(
                        previous_read,
                        STDIN_FILENO
                    ) < 0)
                {
                    perror("dup2 stdin");
                    _exit(127);
                }
            }


            /*
             * ------------------------------------------------
             * OUTPUT TO NEXT PIPE
             *
             * For commands before the last:
             *
             *     pipefd[1] -> STDOUT
             * ------------------------------------------------
             */
            if (i < command_count - 1)
            {
                if (dup2(
                        pipefd[1],
                        STDOUT_FILENO
                    ) < 0)
                {
                    perror("dup2 stdout");
                    _exit(127);
                }
            }


            /*
             * ------------------------------------------------
             * Close original previous pipe descriptor.
             * ------------------------------------------------
             */
            if (previous_read != -1)
            {
                close(previous_read);
            }


            /*
             * ------------------------------------------------
             * Close pipe descriptors after dup2().
             * ------------------------------------------------
             */
            if (i < command_count - 1)
            {
                close(pipefd[0]);
                close(pipefd[1]);
            }


            /*
             * ------------------------------------------------
             * Execute built-in inside child if necessary.
             *
             * Example:
             *
             *     echo hello | wc -w
             * ------------------------------------------------
             */
            if (is_builtin(
                    &pipeline->commands[i]
                ))
            {
                int builtin_status;

                builtin_status = execute_builtin(
                    &pipeline->commands[i]
                );

                if (builtin_status < 0)
                {
                    _exit(1);
                }

                _exit(builtin_status);
            }


            /*
             * ------------------------------------------------
             * Execute external command.
             * ------------------------------------------------
             */
            execvp(
                pipeline->commands[i].argv[0],
                pipeline->commands[i].argv
            );


            /*
             * execvp() failed.
             */
            perror(
                pipeline->commands[i].argv[0]
            );

            _exit(127);
        }


        /*
         * ====================================================
         * PARENT PROCESS
         * ====================================================
         */


        /*
         * ----------------------------------------------------
         * Parent no longer needs previous read end.
         * ----------------------------------------------------
         */
        if (previous_read != -1)
        {
            close(previous_read);
            previous_read = -1;
        }


        /*
         * ----------------------------------------------------
         * If this is not the last command:
         *
         * keep the READ end for the next command.
         *
         * close WRITE end.
         * ----------------------------------------------------
         */
        if (i < command_count - 1)
        {
            close(pipefd[1]);

            previous_read = pipefd[0];
        }
    }


    /*
     * ========================================================
     * Close final previous read descriptor.
     * ========================================================
     */
    if (previous_read != -1)
    {
        close(previous_read);
    }


    /*
     * ========================================================
     * WAIT FOR ALL CHILDREN
     * ========================================================
     */
    for (i = 0; i < command_count; i++)
    {
        int status;


        if (waitpid(
                pids[i],
                &status,
                0
            ) < 0)
        {
            perror("waitpid");
            continue;
        }


        /*
         * Save status of LAST command.
         */
        if (i == command_count - 1)
        {
            if (WIFEXITED(status))
            {
                last_status =
                    WEXITSTATUS(status);
            }
            else if (WIFSIGNALED(status))
            {
                last_status =
                    128 + WTERMSIG(status);
            }
        }
    }


    /*
     * Free PID array.
     */
    free(pids);


    /*
     * Return status of last command.
     */
    return last_status;
}
