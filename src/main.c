#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "token.h"
#include "lexer.h"
#include "history.h"
#include "parser.h"
#include "expand.h"
#include "executor.h"


/*
 * ============================================================
 * SHELLFORGE BANNER
 * ============================================================
 */
static void print_banner(void)
{
    printf("================================\n");
    printf("          Shellforge\n");
    printf("   A Unix Style Shell written in C\n");
    printf("================================\n");
}


/*
 * ============================================================
 * MAIN
 * ============================================================
 */
int main(void)
{
    char *line;

    token_list_t tokens;
    pipeline_t pipeline;

    int status;


    /*
     * --------------------------------------------------------
     * Initialize Shellforge history.
     * --------------------------------------------------------
     */
    history_init();


    /*
     * --------------------------------------------------------
     * Display shell banner.
     * --------------------------------------------------------
     */
    print_banner();


    /*
     * ========================================================
     * MAIN SHELL LOOP
     * ========================================================
     */
    while (1)
    {
        /*
         * ----------------------------------------------------
         * Read command from terminal.
         *
         * readline() provides:
         *
         *   - command line editing
         *   - Backspace
         *   - Arrow keys
         *   - UP arrow history
         *   - Ctrl+D detection
         * ----------------------------------------------------
         */
        line = readline("shellforge$ ");


        /*
         * ----------------------------------------------------
         * Ctrl+D / EOF
         * ----------------------------------------------------
         */
        if (line == NULL)
        {
            printf("\n");
            break;
        }


        /*
         * ----------------------------------------------------
         * Ignore empty command.
         * ----------------------------------------------------
         */
        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }


        /*
         * ----------------------------------------------------
         * Add command to readline history.
         *
         * This allows:
         *
         *     UP arrow
         *
         * to show previous commands.
         * ----------------------------------------------------
         */
        add_history(line);


        /*
         * ----------------------------------------------------
         * Add command to Shellforge history.
         * ----------------------------------------------------
         */
        history_add(line);


        /*
         * ====================================================
         * LEXICAL ANALYSIS
         * ====================================================
         *
         * Convert input string into tokens.
         *
         * Example:
         *
         *     ls -l | grep .c
         *
         * becomes approximately:
         *
         *     WORD ls
         *     WORD -l
         *     PIPE |
         *     WORD grep
         *     WORD .c
         * ----------------------------------------------------
         */
        lexer(
            line,
            &tokens
        );


        /*
         * ----------------------------------------------------
         * Token debug output is disabled.
         *
         * Uncomment this line if you want to debug lexer.
         * ----------------------------------------------------
         */
        /* token_print(&tokens); */


        /*
         * ====================================================
         * PARSING
         * ====================================================
         *
         * Convert tokens into a pipeline structure.
         * ----------------------------------------------------
         */
        if (!parse(
                &tokens,
                &pipeline
            ))
        {
            /*
             * Parsing failed.
             */
            free(line);
            continue;
        }


        /*
         * ----------------------------------------------------
         * Pipeline debug output is disabled.
         *
         * Uncomment for debugging.
         * ----------------------------------------------------
         */
        /* pipeline_print(&pipeline); */


        /*
         * ====================================================
         * VARIABLE / ENVIRONMENT EXPANSION
         * ====================================================
         */
        expand_variables(
            &pipeline
        );


        /*
         * ====================================================
         * EXECUTION
         * ====================================================
         *
         * Milestone 4.1:
         *
         *     Single command:
         *         execute_command()
         *
         *     Multiple commands:
         *         execute_pipeline()
         *
         * execute_pipeline() handles:
         *
         *     pipe()
         *     fork()
         *     dup2()
         *     execvp()
         *     close()
         *     waitpid()
         *
         * as required for pipeline execution.
         * ====================================================
         */
        status = execute_pipeline(
            &pipeline
        );


        /*
         * ====================================================
         * EXIT COMMAND
         * ====================================================
         *
         * The built-in exit command returns status 1.
         *
         * We only treat it as shell termination when it is
         * a single command.
         *
         * This avoids incorrectly terminating the shell for
         * something such as:
         *
         *     exit | wc
         * ====================================================
         */
        if (
            pipeline.command_count == 1 &&
            pipeline.commands[0].argc > 0 &&
            strcmp(
                pipeline.commands[0].argv[0],
                "exit"
            ) == 0 &&
            status == 1
        )
        {
            /*
             * Free pipeline memory.
             */
            pipeline_free(
                &pipeline
            );


            /*
             * Free input line.
             */
            free(line);


            /*
             * Leave shell loop.
             */
            break;
        }


        /*
         * ====================================================
         * CLEAN UP PIPELINE
         * ====================================================
         */
        pipeline_free(
            &pipeline
        );


        /*
         * ====================================================
         * CLEAN UP INPUT
         * ====================================================
         */
        free(line);
    }


    /*
     * ========================================================
     * CLEAN UP READLINE HISTORY
     * ========================================================
     */
    clear_history();


    return 0;
}
