/*
    Helper functions to partially de-clutter main.
*/

#ifndef HELPER_H
#define HELPER_H
#include <termios.h>

static struct termios old, current;

/* Initialize new terminal i/o settings. author -niko */
void initTermios(int echo);

/* Restore old terminal i/o settings. author -niko */
void resetTermios(void);

/* Read 1 character - echo defines echo mode. author -niko */
char getch_(int echo);

/* Read 1 character without echo. author -niko */
char getch(void);

/* Read 1 character with echo. author -niko */
char getche(void);

/* Returns a 1 if the given string starts with that char looking_for, 0 otherwise*/
int starts_with(char *line, char looking_for);

/* Removes all leading spaces in the given string*/
char* clean_string(char *source, int length);

/* Given two arrays of strings, copies the given indices of source into the destination */
void copy_arr(char **source, char ***dest, int start, int end);

/* Frees memory allocated to an array of char* (a string array) - WARNING, does not ensure the 
memory being freed has actually been allocated.*/
void free_arr(char ***arr, int length);
#endif
