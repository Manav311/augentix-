#include "tutk_bubble_sort.h"

/* Function to print nodes in a given linked list */
void printList(Node *start)
{
	printf("enter print list:");
	Node *temp = start;
	printf("\n");
	while (temp != NULL) {
		printf("save : %lu\n", temp->data.key);
		temp = temp->next;
	}
}

/* Bubble sort the given linked list */
void bubbleSort(Node *start)
{
	int swapped;
	Node *ptr1;
	Node *lptr = NULL;

	/* Checking for empty list */
	if (start == NULL)
		return;

	do {
		swapped = 0;
		ptr1 = start;

		while (ptr1->next != lptr) {
			if (ptr1->data.key > ptr1->next->data.key) {
				swap(ptr1, ptr1->next);
				swapped = 1;
			}
			ptr1 = ptr1->next;
		}
		lptr = ptr1;
	} while (swapped);
}

/* function to swap data of two nodes a and b*/
void swap(Node *a, Node *b)
{
	KeyPair temp = a->data;
	a->data.key = b->data.key;
	b->data = temp;
}