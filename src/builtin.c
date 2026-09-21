#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtin.h"


/*
 * =========================================================
 * CHECK WHETHER COMMAND IS BUILT-IN
 * =========================================================
 */
int is_builtin(const command_t *cmd)
{
    if (cmd == NULL || cmd->argc <= 0)
    {
        return 0;
    }

    if (strcmp(cmd->argv[0], "cd") == 0)
    {
        return 1;
    }

    if (strcmp(cmd->argv[0], "pwd") == 0)
    {
        return 1;
    }

    if (strcmp(cmd->argv[0], "echo") == 0)
    {
        return 1;
    }

    if (strcmp(cmd->argv[0], "exit") == 0)
    {
        return 1;
    }

    return 0;
}


/*
 * =========================================================
 * BUILT-IN: cd
 *
 * Examples:
 *
 *     cd
 *     cd ..
 *     cd /home/syam
 *     cd ~
 * =========================================================
 */
static int builtin_cd(const command_t *cmd)
{
    const char *directory;


    /*
     * cd with no argument:
     * go to HOME.
     */
    if (cmd->argc == 1)
    {
        directory = getenv("HOME");

        if (directory == NULL)
        {
            fprintf(
                stderr,
                "cd: HOME not set\n"
            );

            return -1;
        }
    }


    /*
     * cd with one argument.
     */
    else if (cmd->argc == 2)
    {
        directory = cmd->argv[1];


        /*
         * Support:
         *
         *     cd ~
         */
        if (strcmp(directory, "~") == 0)
        {
            directory = getenv("HOME");

            if (directory == NULL)
            {
                fprintf(
                    stderr,
                    "cd: HOME not set\n"
                );

                return -1;
            }
        }
    }


    /*
     * Too many arguments.
     */
    else
    {
        fprintf(
            stderr,
            "cd: too many arguments\n"
        );

        return -1;
    }


    /*
     * Change directory.
     */
    if (chdir(directory) != 0)
    {
        perror("cd");
        return -1;
    }


    return 0;
}


/*
 * =========================================================
 * BUILT-IN: pwd
 *
 * IMPORTANT:
 *
 * We do NOT use PATH_MAX.
 *
 * getcwd(NULL, 0) automatically allocates enough memory.
 * =========================================================
 */
static int builtin_pwd(const command_t *cmd)
{
    char *cwd;


    /*
     * pwd should not receive arguments.
     */
    if (cmd->argc > 1)
    {
        fprintf(
            stderr,
            "pwd: too many arguments\n"
        );

        return -1;
    }


    /*
     * Get current working directory.
     *
     * No PATH_MAX required.
     */
    cwd = getcwd(NULL, 0);


    /*
     * getcwd() failed.
     */
    if (cwd == NULL)
    {
        perror("pwd");
        return -1;
    }


    /*
     * Print directory.
     */
    printf(
        "%s\n",
        cwd
    );


    /*
     * getcwd(NULL, 0) allocated memory.
     * We must free it.
     */
    free(cwd);


    return 0;
}


/*
 * =========================================================
 * BUILT-IN: echo
 *
 * Example:
 *
 *     echo Hello World
 *
 * Output:
 *
 *     Hello World
 * =========================================================
 */
static int builtin_echo(const command_t *cmd)
{
    int i;


    for (i = 1; i < cmd->argc; i++)
    {
        printf(
            "%s",
            cmd->argv[i]
        );


        /*
         * Space between arguments.
         */
        if (i < cmd->argc - 1)
        {
            printf(" ");
        }
    }


    printf("\n");


    return 0;
}


/*
 * =========================================================
 * BUILT-IN: exit
 * =========================================================
 */
static int builtin_exit(const command_t *cmd)
{
    /*
     * exit should not receive arguments
     * in this version.
     */
    if (cmd->argc > 1)
    {
        fprintf(
            stderr,
            "exit: too many arguments\n"
        );

        return -1;
    }


    /*
     * Return special value to main().
     */
    return 1;
}


/*
 * =========================================================
 * EXECUTE BUILT-IN
 * =========================================================
 */
int execute_builtin(const command_t *cmd)
{
    if (cmd == NULL || cmd->argc <= 0)
    {
        return -1;
    }


    /*
     * cd
     */
    if (strcmp(cmd->argv[0], "cd") == 0)
    {
        return builtin_cd(cmd);
    }


    /*
     * pwd
     */
    if (strcmp(cmd->argv[0], "pwd") == 0)
    {
        return builtin_pwd(cmd);
    }


    /*
     * echo
     */
    if (strcmp(cmd->argv[0], "echo") == 0)
    {
        return builtin_echo(cmd);
    }


    /*
     * exit
     */
    if (strcmp(cmd->argv[0], "exit") == 0)
    {
        return builtin_exit(cmd);
    }


    return -1;
}
