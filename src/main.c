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

static void print_banner(void)
{
    printf("================================\n");
    printf("          Shellforge\n");
    printf("   A Unix Style Shell written in C\n");
    printf("================================\n");
}

int main(void)
{
    char *line;

    token_list_t tokens;
    pipeline_t pipeline;

    history_init();

    print_banner();

    while (1)
    {
        line = readline("shellforge$ ");

        /*
         * Ctrl+D
         */
        if (line == NULL)
        {
            printf("\nGoodbye!\n");
            break;
        }

        /*
         * Empty line
         */
        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }

        /*
         * Add to readline history.
         * This enables UP arrow.
         */
        add_history(line);

        /*
         * Add to our history.
         */
        history_add(line);

        /*
         * Exit
         */
        if (strcmp(line, "exit") == 0)
        {
            free(line);
            printf("Exiting...\n");
            break;
        }

        /*
         * history
         */
        if (strcmp(line, "history") == 0)
        {
            history_print();

            free(line);
            continue;
        }

        /*
         * LEXER
         */
        lexer(line, &tokens);

        /*
         * Print tokens
         */
        token_print(&tokens);

        /*
         * PARSER
         */
        if (!parse(&tokens, &pipeline))
        {
            free(line);
            continue;
        }

        /*
         * EXPAND VARIABLES
         */
        expand_variables(&pipeline);

        /*
         * Print pipeline
         */
        pipeline_print(&pipeline);

        /*
         * Free pipeline memory
         */
        pipeline_free(&pipeline);

        free(line);
    }

    clear_history();

    return 0;
}
