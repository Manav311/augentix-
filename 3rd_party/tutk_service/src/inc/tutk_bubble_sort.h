#ifndef BUBBLESORT_HEADER
#define BUBBLESORT_HEADER

#include "tutk_linked_list.h"

/* Function to insert a node at the beginning of a linked list */
void insertAtTheBegin(Node **start_ref, int data);

/* Function to bubble sort the given linked list */
void bubbleSort(Node *start);

/* Function to swap data of two nodes a and b*/
void swap(Node *a, Node *b);

/* Function to print nodes in a given linked list */
void printList(Node *start);

#endif