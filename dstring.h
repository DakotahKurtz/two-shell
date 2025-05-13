/*
    d_string => Dakotah's String
    To make reading characters one at a time from the command prompt tolerable. Provides
    adding a char to the end of the string, removing the char at a given index, and 
    "adding" two d_strings together.
*/

#ifndef DSTRING_H
#define DSTRING_H

typedef struct d_string {
    char* arr;
    int size; // currently in use
    int max; // available memory
} dstring;

/* Add char c to the end of the given string */
void add_end(dstring* string, char c);

/* Remove the given index from the string */
void remove_dstring_index(dstring* string, int index);

/* Adds the source dstring to the end of the dest dstring
    dest = 'apple', source = 'sauce'
    add_arr_end(dest, source)
    dest = 'applesauce' source = 'sauce'
*/
void add_arr_end(dstring* dest, dstring* source);

/*
    Release memory associated with d_string and reset struct variables
*/
void clear_string(dstring* string);

#endif

