/*
    Implementation of a doubly linked_list (of char*'s) for use in the shell. 
    Tracks the history of commands run (or attempted to be run) on the shell. 
    Includes several unused functions because I was procrastinating actually doing the assignment.
*/

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node
{
    char *val;
    struct node *next;
    struct node *previous;
} node;

typedef struct linked_list
{
    node *head;
    node *tail;
    int length;
} llist;

/*
    Print linked list, using the given char as a "spacer" between elements

*/
void print(llist* list, char);

/*
    Print linked list in reverse, using the given char as a "spacer" between elements
*/
void print_rev(llist* list, char);

/*
    Adds the given char* to the front of the list.
*/
void add_first(llist *list, char *v);

/*
    Adds the given char* to the end of the linked list, AND RELEASES THE MEMORY -- use with caution
*/
void add_last(llist *list, char *v);

/*
    Removes the given index from the list, if the index is within bounds of the list.
*/
void remove_index(llist *list, int index);

/*
    Empty the list of all nodes and release memory associated with each node.
*/
void empty_list(llist *list);
/*
    Return the value of the list at the given index, if the index is within the bounds of the list.
*/
char* get(llist* list, int index);

/*
    Returns 1 if any element of the list begins with the char* passed to value
*/
int contains(llist* list, char* value);


#endif

