#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "linked_list.h"

void empty_list(llist* list) {
    if (list -> length <= 0) {
        return;
    }
    node* current = list -> head;
    while (1) {
        if (current -> next == NULL) { // we've reached the end
            break;
        }
        node* next = current -> next; // where we're going
        free(current->val); // free current
        current -> next = NULL;
        current -> previous = NULL;
        free(current);
        current = next;
    } // so free the last node
        free(current->val);
        current -> next = NULL;
        current -> previous = NULL;
        free(current);
        list -> head = NULL;
        list -> tail = NULL;
        list -> length = 0;
}

void print(llist* list, char spacer)
{
    int i = 0;    
    if (list->head != NULL)
    {
        node *current = list->head;
        while (1)
        {
            printf("%d  %s%c", i++, current->val, spacer);
            if (current->next == NULL)
            {
                break;
            }
            current = current->next;
        }
    }
}

void print_rev(llist* list, char spacer)
{
    int i = (list -> length)--;
    if (list->tail != NULL)
    {
        node *current = list->tail;
        while (1)
        {
            printf("%d  %s%c", i--, current->val, spacer);
            if (current->previous == NULL)
            {
                break;
            }
            current = current->previous;
        }
    }
}

void add_first(llist *list, char *v)
{
        node *n = malloc(sizeof(node));

        char c;
        // count size of new node's value
        int i = 0;
        do {
            c = v[i++];

        } while(c != '\0');
        // new node is set up
        char* copy = malloc(sizeof(char) * i);
        strcpy(copy, v);
        n->val = copy;

    if (list->length == 0) // now to add it to the list
    {
        list->head = n;
        list->tail = n;
    }
    else
    {
        node *old = list->head;
        list->head = n;
        list->head->next = old;
        old->previous = list->head;
    }
    (list->length)++;

}

void add_last(llist *list, char *v)
{
        node *n = malloc(sizeof(node));
        char c;
        int i = 0;
        // count size of new node's value
        do {
            c = v[i++];

        } while(c != '\0');
        // set up new node
        char* copy = malloc(sizeof(char) * i);
        strcpy(copy, v);
        n->val = copy;
    if (list->length == 0) // add it to list
    {
        list->head = n;
        list->tail = n;
    }
    else
    {
        node *old = list->tail;
        list->tail = n;
        list->tail->previous = old;
        old->next = list->tail;
    }
    (list -> length)++;
    // needing to free v here and not in add_first is because I realized last minute I had a memory leak here,
    // and this function actually only gets used ONCE, so rather than restructure the night before the 
    // assignment is do, I'm doing this.
    free(v);
}

int contains(llist* list, char* value) {
    // if a value is going to match for the first 'n' characters, it'll match for the first
    // char. Since we mark it as a 'hit' as soon as the user types a char that matches one from history,
    // we really only need to compare the first character of value with each node's value
    node* n = list -> head; 
    int i = 0;
    while (n != NULL) {
        if (n -> val[0] == value[0]) {
            return i;
        }
        i++;
        n = n -> next;
    }
    return -1;
}

void remove_index(llist *list, int index)
{
    if (index >= list->length)
    {
        return;
    }
    else
    {
        if (index == 0) // node 2 becomes head
        { // remove head
            node *old = list->head;
            list->head = old->next;
            list->head->previous = NULL;
            old->next = NULL;
            free(old);
        }
        else if ((index + 1) == list->length) // second to last node becomes tail
        { // remove tail
            node *old = list->tail;
            list->tail = old->previous;
            list->tail->next = NULL;
            old->previous = NULL;
            free(old);
        }
        else // increment through the list to find the appropriate node
        {
            int i = 0;
            node *current = list->head;
            while (i != index)
            {
                current = current->next;
                i++;
            }
            // don't forget to free char*
            node *previous = current->previous;
            previous->next = current->next;
            node *next = current->next;
            next->previous = current->previous;
            current->previous = NULL;
            current->next = NULL;
            free(current);
        }
        list -> length--; // <- DON'T FORGET THIS
    }
}

char* get(llist* list, int index) {
    
    if (index < 0 || index >= (list -> length)) {
        return NULL;
    }

    int i = 0;
    node* n = list -> head;
    while (i < index) { // increment through, find the right index
        n = n -> next;
        i++;
    }
    return n -> val;
}


