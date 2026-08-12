#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "token.h"
#include "lexer.h"
#include "history.h"

static void print_banner(void)
{
    printf("================================\n");
    printf("          Shellforge\n");
    printf("   A Unix Style Shell written in C\n");
    printf("================================\n");
}

int main(void)
{
    char *input;
    token_list_t tokens;

    history_init();

    print_banner();

    while (1)
    {
        /*
         * readline() automatically supports:
         * UP arrow    -> previous command
         * DOWN arrow  -> next command
         * LEFT/RIGHT  -> move cursor
         * BACKSPACE   -> delete
         */
        input = readline("shellforge$ ");

        /*
         * Ctrl+D / EOF
         */
        if (input == NULL)
        {
            printf("\nExiting...\n");
            break;
        }

        /*
         * Ignore empty input.
         */
        if (strlen(input) == 0)
        {
            free(input);
            continue;
        }

        /*
         * exit command
         */
        if (strcmp(input, "exit") == 0)
        {
            free(input);
            printf("Exiting...\n");
            break;
        }

        /*
         * Add command to GNU Readline history.
         * This is what makes UP arrow work.
         */
        add_history(input);

        /*
         * Add command to our Shellforge history.
         */
        history_add(input);

        /*
         * history command:
         * Print complete history.
         */
        if (strcmp(input, "history") == 0)
        {
            history_print();

            free(input);
            continue;
        }

        /*
         * Tokenize command.
         */
        lexer(input, &tokens);

        /*
         * Display tokens.
         */
        token_print(&tokens);

        free(input);
    }

    /*
     * Free GNU Readline history.
     */
    clear_history();

    return 0;
}
