#include <stdlib.h>
#include <stdio.h>
#include "dstring.h"

void add_end(dstring* string, char c) {
    // first character, need to allocate memory and track how much memory we have.
    if (string -> size == 0) {
        string -> max = 5;
        string->arr = malloc(sizeof(char) * string -> max);
    } else if ((string -> size) + 2 == string -> max) {
        // we're approaching the limits of memory in string -> arr
        string -> max = (string -> size) * 5;
        // so increase it, copy the info over into larger array
        char* temp = malloc(sizeof(char) * string -> max);
        for (int i = 0; i < string -> size; i++) {
            temp[i] = string -> arr[i];
        }
        free(string -> arr);
        string -> arr = temp;
    }
    (string -> size)++;
    // always make sure the array is null terminated
    string -> arr[(string -> size) - 1] = c;
    string -> arr[(string -> size)] = '\0';
}

void remove_dstring_index(dstring* string, int index) {
    if (string -> size <= 0 || index >= string -> size) {
        return;
    }
    // going to need a whole new array
    char* new = malloc(sizeof(char) * string -> size);
    int i = 0;
    int j = 0;
    while (1) {
        if (i == index) { // skip the index we're removing
            i++;
        }

        if (i < string -> size) {
            new[j] = string->arr[i];
        } else {
            break;
        }
        i++;
        j++;
    }
    (string -> size)--;
    free(string->arr);
    string -> arr = new;
}

void add_arr_end(dstring* dest, dstring* source) {
    if (source -> size <= 0) {
        return;
    }
    char* new = malloc(sizeof(char) * (dest -> size + source -> size));
    int i = 0;
    // copy dest into the new arr
    while (i < dest -> size) {
        new[i] = dest -> arr[i];
        i++;
        if (dest -> arr[i] == '\0') { 
            // don't null terminate in the middle
            break;
        }
    }
    int j = 0;
    // copy source into the new array
    while (j < source -> size) {
        new[i] = source -> arr[j];
        j++;
        i++;
    } 
    if (new[i] != '\0') {
        new[i] = '\0';
    }

    free(dest->arr);
    dest -> size = i;
    dest -> arr = new;
}

void clear_string(dstring* string) {
    free(string -> arr);
    string -> size = 0;
    string -> max = 0;
}

