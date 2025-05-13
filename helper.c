#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "helper.h"

/*EVERYTHING FROM HERE*/
static struct termios old, current;

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
    tcgetattr(0, &old);         /* grab old terminal i/o settings */
    current = old;              /* make new settings same as old settings */
    current.c_lflag &= ~ICANON; /* disable buffered i/o */
    if (echo)
    {
        current.c_lflag |= ECHO; /* set echo mode */
    }
    else
    {
        current.c_lflag &= ~ECHO; /* set no echo mode */
    }
    tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
    tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo)
{
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

/* Read 1 character without echo */
char getch(void)
{
    return getch_(0);
}

/* Read 1 character with echo */
char getche(void)
{
    return getch_(1);
}

/* TO HERE IS SHAMELESSY STOLEN - I miss Java KeyListeners */

int starts_with(char *line, char looking_for)
{
    for (int i = 0;; i++)
    {
        if (line[i] == looking_for)
        {
            return 1;
        }
        else if (line[i] != ' ')
        {
            return 0;
        }
    }
}

char *clean_string(char *source, int length)
{
    int i = 0;
    while (source[i] == ' ') // skip all leading spaces
    {
        i++;
    }
    char *cleaned = malloc(sizeof(char) * ((length - i) + 1));
    // copy source into new array
    for (int j = 0; j + i < length; j++)
    {
        cleaned[j] = source[i + j];
    }

    return cleaned;
}




void copy_arr(char **source, char ***dest, int start, int end)
{
    // +1 to have space for NULL in last arg
    *dest = malloc(((end - start) + 1) * sizeof(char *));
    for (int i = 0; i + start < end; i++)
    {
        (*dest)[i] = malloc(sizeof(char *));
        strcpy((*dest)[i], source[i + start]);
    }
}


void free_arr(char ***arr, int length)
{
    for (int i = 0; i < length; i++)
    {
        free((*arr)[i]);
    }
    free((*arr));
}
