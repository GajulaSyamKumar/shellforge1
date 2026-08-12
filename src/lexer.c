#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"
#include "token.h"

static void add_word(token_list_t *list, char *word, int *length)
{
    if (*length == 0)
        return;

    word[*length] = '\0';

    token_add(list, TOKEN_WORD, word);

    *length = 0;
    word[0] = '\0';
}

void lexer(const char *input, token_list_t *list)
{
    int i = 0;
    int length = 0;

    char word[MAX_TOKEN_LEN];

    token_list_init(list);

    if (input == NULL)
        return;

    while (input[i] != '\0')
    {
        char c = input[i];

        /* End of input */
        if (c == '\n')
        {
            add_word(list, word, &length);
            break;
        }

        /* Spaces and tabs */
        if (isspace((unsigned char)c))
        {
            add_word(list, word, &length);
            i++;
            continue;
        }

        /* Pipe */
        if (c == '|')
        {
            add_word(list, word, &length);

            token_add(list, TOKEN_PIPE, "|");

            i++;
            continue;
        }

        /* Input redirection */
        if (c == '<')
        {
            add_word(list, word, &length);

            token_add(list, TOKEN_INPUT, "<");

            i++;
            continue;
        }

        /* Output redirection */
        if (c == '>')
        {
            add_word(list, word, &length);

            if (input[i + 1] == '>')
            {
                token_add(list, TOKEN_APPEND, ">>");
                i += 2;
            }
            else
            {
                token_add(list, TOKEN_OUTPUT, ">");
                i++;
            }

            continue;
        }

        /* Background */
        if (c == '&')
        {
            add_word(list, word, &length);

            token_add(list, TOKEN_BACKGROUND, "&");

            i++;
            continue;
        }

        /*
         * Single quoted string
         */
        if (c == '\'')
        {
            i++;

            while (input[i] != '\0' && input[i] != '\'')
            {
                if (length < MAX_TOKEN_LEN - 1)
                {
                    word[length++] = input[i];
                }

                i++;
            }

            if (input[i] == '\'')
            {
                i++;
            }

            continue;
        }

        /*
         * Double quoted string
         */
        if (c == '"')
        {
            i++;

            while (input[i] != '\0' && input[i] != '"')
            {
                if (input[i] == '\\' && input[i + 1] != '\0')
                {
                    i++;

                    if (length < MAX_TOKEN_LEN - 1)
                    {
                        word[length++] = input[i];
                    }

                    i++;
                    continue;
                }

                if (length < MAX_TOKEN_LEN - 1)
                {
                    word[length++] = input[i];
                }

                i++;
            }

            if (input[i] == '"')
            {
                i++;
            }

            continue;
        }

        /*
         * Backslash escape
         */
        if (c == '\\')
        {
            if (input[i + 1] != '\0')
            {
                i++;

                if (length < MAX_TOKEN_LEN - 1)
                {
                    word[length++] = input[i];
                }

                i++;
            }
            else
            {
                i++;
            }

            continue;
        }

        /*
         * Normal character
         */
        if (length < MAX_TOKEN_LEN - 1)
        {
            word[length++] = c;
        }

        i++;
    }

    add_word(list, word, &length);

    /*
     * Always add END token.
     */
    token_add(list, TOKEN_END, "END");
}
